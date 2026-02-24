//
// Created by Shinnosuke Kawai on 1/25/26.
//

#include "redis/redis_lib.h"

namespace Database {
    static void PreparePrams(const std::string& cmd, const std::deque<std::string>& params, std::vector<const char*>& argv, std::vector<size_t>& argvlen) {
        argv.clear();
        argvlen.clear();
        argv.reserve(params.size() + 1);
        argvlen.reserve(params.size() + 1);
        argv.push_back(cmd.c_str());
        argvlen.emplace_back(cmd.size());
        for (const auto& param : params) {
            argv.push_back(param.c_str());
            argvlen.emplace_back(param.size());
        }
    }

    std::expected<void, Core::Database::ConnectionError> Redis::connect() const noexcept {
        using Core::Database::ConnectionError;
        constexpr timeval timeout = {1, 500000};
        redisContext* row_ctx = redisConnectWithTimeout(m_config.host.c_str(), m_config.port, timeout);

        if (!row_ctx)
            return std::unexpected(ConnectionError::ConnectionFailed("Redis connection failed"));

        UniqueContext unique_context(row_ctx);

        if (unique_context->err)
            return std::unexpected(ConnectionError::ConnectionFailed(unique_context->errstr));

        if (m_config.password) {
            void* row_reply = redisCommand(unique_context.get(), "AUTH %s", m_config.password->c_str());
            if (!row_reply) {
                return std::unexpected(ConnectionError::AuthFailed("Redis Auth failed"));
            }
            const UniqueReply reply(static_cast<redisReply*>(row_reply));
            if (reply->type == REDIS_REPLY_STRING && strcmp(reply->str, "OK") != 0)
            {
                return std::unexpected(ConnectionError::AuthFailed(reply->str));
            }
        }

        m_context = std::move(unique_context);
        m_worker_thread = std::jthread([this](const std::stop_token& st) mutable {worker_thread_func(st);});

        return {};
    }

    // Helper struct for the overload pattern (C++17)
    template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };

    void Redis::worker_thread_func(const std::stop_token &st) const noexcept {
        while (!st.stop_requested()) {
            QueueItem item{};
            {
                std::unique_lock ul(m_mutex);
                m_cv.wait(ul, [this, st]{ return st.stop_requested() || !m_requests.empty();});
                if (st.stop_requested()) {
                    break;
                }
                item = std::move(m_requests.front());
                m_requests.pop();
            }
            std::vector<const char*> argv;
            std::vector<size_t> argvlen;
            std::visit(overloaded {
                [&](const RedisRequest& redis_req){
                    PreparePrams(redis_req.cmd, redis_req.params, argv, argvlen);
                    void* raw = redisCommandArgv(m_context.get(), static_cast<int>(argv.size()), argv.data(), argvlen.data());
                    if (!raw) {
                        redis_req.on_error(RedisError::ReplyNull());
                        return;
                    }
                    UniqueReply reply(static_cast<redisReply*>(raw));
                    if (reply->type == REDIS_REPLY_ERROR) {
                        redis_req.on_error(RedisError::ReplyError(reply->str));
                        return;
                    }
                    redis_req.on_success(std::move(reply));
                },
                [&](PipelineRequest& pipeline_req) {
                    size_t appended = 0;
                    std::deque<UniqueReply> replies;
                    std::optional<RedisError> err_opt;
                    for (const auto& [cmd, params] : pipeline_req.commands) {
                        PreparePrams(cmd, params, argv, argvlen);
                        if (redisAppendCommandArgv(m_context.get(), static_cast<int>(argv.size()),argv.data(),argvlen.data()) != REDIS_OK)
                        {
                            err_opt.emplace(RedisError::AppendFailed());
                            pipeline_req.commands.clear();
                            break;
                        }
                        appended++;
                    }
                    for (size_t i = 0; i < appended; ++i) {
                        void* raw = nullptr;
                        if (redisGetReply(m_context.get(), &raw) != REDIS_OK || !raw) {
                            err_opt.emplace(RedisError::ReplyNull());
                            break;
                        }
                        UniqueReply reply(static_cast<redisReply*>(raw));
                        if (reply->type == REDIS_REPLY_ERROR) {
                            err_opt.emplace(RedisError::ReplyError(reply->str));
                            break;
                        }
                        replies.emplace_back(std::move(reply));
                    }
                    try {
                        if (err_opt) {
                            pipeline_req.promise_ptr.set_value(std::unexpected(std::move(*err_opt)));
                        } else {
                            pipeline_req.promise_ptr.set_value(std::move(replies));
                        }
                    } catch (std::future_error& future_error) {
                        std::printf("Redis pipeline error: %s\n", future_error.what());
                    }
                },
            }, item);
        }
    }
}
