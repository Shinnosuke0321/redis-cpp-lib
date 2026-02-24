//
// Created by Shinnosuke Kawai on 11/3/25.
//

#include "redis.h"
#include <util/helpers.h>

namespace Database {
    std::expected<std::unique_ptr<Redis>,Core::Database::ConnectionError> Redis::ConnectionFactory() noexcept {
        using Core::Database::ConnectionError;
        const char* host = std::getenv("REDIS_HOST");
        const char* port = std::getenv("REDIS_PORT");
        char* password = std::getenv("REDIS_PASSWORD");
        if (!host || !port || !password)
            return std::unexpected(ConnectionError::MissingConfig("Redis connection parameters not provided"));

        ConnectionConfig config{};
        config.host = host;
        config.port = std::stoi(port);
        config.password = password;

        auto redis = std::make_unique<Redis>(std::move(config));
        if (std::expected<void, ConnectionError> result = redis->connect(); !result)
            return std::unexpected(result.error());
        return redis;
    }

    Redis::~Redis() {
        m_worker_thread.request_stop();
        m_cv.notify_all();
    }

    void Redis::execute_command_async(std::string_view cmd, std::string_view key, ResultCallback &&callback, ErrorCallback &&err_callback) const noexcept {
        RedisRequest request{};
        request.cmd = std::string(cmd);
        request.params.emplace_back(key);
        request.on_success = std::move(callback);
        request.on_error = std::move(err_callback);
        {
            std::lock_guard sl(m_mutex);
            m_requests.emplace(std::move(request));
        }
        m_cv.notify_one();
    }

    void Redis::execute_command_async(const std::string_view cmd, const std::string_view key, const std::string_view str_val, ResultCallback &&callback, ErrorCallback &&err_callback) const noexcept {
        RedisRequest request{};
        request.cmd = std::string(cmd);
        request.params.emplace_back(key);
        request.params.emplace_back(str_val);
        request.on_success = std::move(callback);
        request.on_error = std::move(err_callback);
        {
            std::lock_guard sl(m_mutex);
            m_requests.emplace(std::move(request));
        }
        m_cv.notify_one();
    }

    void Redis::execute_command_async(const std::string_view cmd, const std::string_view key, std::unordered_map<std::string, std::string>&& map, ResultCallback &&callback, ErrorCallback &&err_callback) const noexcept {
        RedisRequest request{};
        request.cmd = std::string(cmd);
        request.params.emplace_back(key);
        for (auto& [k, v] : map) {
            request.params.push_back(k);
            request.params.emplace_back(std::move(v));
        }
        request.on_success = std::move(callback);
        request.on_error = std::move(err_callback);
        {
            std::unique_lock sl(m_mutex);
            m_requests.emplace(std::move(request));
        }
        m_cv.notify_one();
    }

    void Redis::execute_command_async(const std::string_view cmd, const std::string_view key, std::unordered_set<std::string>&& set, ResultCallback &&callback, ErrorCallback &&err_callback) const noexcept {
        RedisRequest request{};
        request.cmd = std::string(cmd);
        request.params.emplace_back(key);
        while (!set.empty()) {
            request.params.emplace_back(std::move(set.extract(set.begin()).value()));
        }
        request.on_success = std::move(callback);
        request.on_error = std::move(err_callback);
        {
            std::unique_lock sl(m_mutex);
            m_requests.emplace(std::move(request));
        }
        m_cv.notify_one();
    }

    std::future<std::expected<Redis::UniqueReply, RedisError> > Redis::execute_command(const std::string_view cmd, const std::string_view key) const noexcept {
        auto prom = std::make_shared<std::promise<std::expected<UniqueReply, RedisError>>>();
        auto future = prom->get_future();
        RedisRequest request{};
        request.cmd = std::string(cmd);
        request.params.emplace_back(key);
        request.on_success = [prom](UniqueReply reply) {
            try {
                prom->set_value(std::move(reply));
            } catch (...) {}
        };
        request.on_error = [prom](const RedisError& err) {
            try {
                prom->set_value(std::unexpected(err));
            } catch (...) {}
        };
        {
            std::lock_guard sl(m_mutex);
            m_requests.emplace(std::move(request));
        }
        m_cv.notify_one();
        return future;
    }

    std::future<std::expected<Redis::UniqueReply, RedisError>> Redis::execute_command(const std::string_view cmd, const std::string_view key, const std::string_view str_val) const noexcept {
        auto prom = std::make_shared<std::promise<std::expected<UniqueReply, RedisError>>>();
        auto future = prom->get_future();
        RedisRequest request{};
        request.cmd = std::string(cmd);
        request.params.emplace_back(key);
        request.params.emplace_back(str_val);
        request.on_success = [prom](UniqueReply reply) {
            try {
                prom->set_value(std::move(reply));
            } catch (...) {}
        };
        request.on_error = [prom](const RedisError& err) {
            try {
                prom->set_value(std::unexpected(err));
            } catch (...) {}
        };
        {
            std::lock_guard sl(m_mutex);
            m_requests.emplace(std::move(request));
        }
        m_cv.notify_one();
        return future;
    }

    std::future<std::expected<Redis::UniqueReply, RedisError> > Redis::execute_command(const std::string_view cmd, const std::string_view key, std::unordered_map<std::string, std::string> &&map) const noexcept {
        auto prom = std::make_shared<std::promise<std::expected<UniqueReply, RedisError>>>();
        auto future = prom->get_future();
        RedisRequest request{};
        request.cmd = std::string(cmd);
        request.params.emplace_back(key);
        for (auto& [k, v] : map) {
            request.params.emplace_back(k);
            request.params.emplace_back(std::move(v));
        }
        request.on_success = [prom](UniqueReply reply) {
            try {
                prom->set_value(std::move(reply));
            } catch (...) {}
        };
        request.on_error = [prom](const RedisError& err) {
            try {
                prom->set_value(std::unexpected(err));
            } catch (...) {}
        };
        {
            std::lock_guard sl(m_mutex);
            m_requests.emplace(std::move(request));
        }
        m_cv.notify_one();
        return future;
    }

    std::future<std::expected<Redis::UniqueReply, RedisError> > Redis::execute_command(const std::string_view cmd, const std::string_view key, std::unordered_set<std::string>&& set) const noexcept {
        auto prom = std::make_shared<std::promise<std::expected<UniqueReply, RedisError>>>();
        auto future = prom->get_future();
        RedisRequest request{};
        request.cmd = std::string(cmd);
        request.params.emplace_back(key);
        while (!set.empty()) {
            request.params.emplace_back(std::move(set.extract(set.begin()).value()));
        }
        request.on_success = [prom](UniqueReply reply) {
            try {
                prom->set_value(std::move(reply));
            } catch (...) {}
        };
        request.on_error = [prom](const RedisError& err) {
            try {
                prom->set_value(std::unexpected(err));
            } catch (...) {}
        };
        {
            std::lock_guard sl(m_mutex);
            m_requests.emplace(std::move(request));
        }
        m_cv.notify_one();
        return future;
    }

    void Redis::append(const std::string_view cmd, const std::string_view key) const noexcept {
        PipelineCommand command{};
        command.cmd = std::string(cmd);
        command.params.emplace_back(key);
        {
            std::lock_guard sl(m_mutex);
            m_pipeline_buffer.emplace_back(std::move(command));
        }
    }

    void Redis::append(const std::string_view cmd, const std::string_view key, std::deque<std::string>&& params) const noexcept {
        PipelineCommand command{};
        command.cmd = std::string(cmd);
        command.params.emplace_back(key);
        for (auto& p : params) {
            command.params.emplace_back(std::move(p));
        }
        {
            std::lock_guard sl(m_mutex);
            m_pipeline_buffer.emplace_back(std::move(command));
        }
    }
    void Redis::append(const std::string_view cmd, const std::string_view key, const std::string_view str_val) const noexcept {
        PipelineCommand command{};
        command.cmd = std::string(cmd);
        command.params.emplace_back(key);
        command.params.emplace_back(str_val);
        {
            std::lock_guard sl(m_mutex);
            m_pipeline_buffer.emplace_back(std::move(command));
        }
    }

    void Redis::append(const std::string_view cmd, std::string_view key, std::unordered_set<std::string>&& set) const noexcept {
        PipelineCommand command{};
        command.cmd = std::string(cmd);
        command.params.emplace_back(key);
        for (auto it = set.begin(); it != set.end();) {
            command.params.emplace_back(std::move(set.extract(++it).value()));
        }
        {
            std::lock_guard sl(m_mutex);
            m_pipeline_buffer.emplace_back(std::move(command));
        }
    }

    void Redis::append(const std::string_view cmd, std::string_view key, std::unordered_map<std::string, std::string>&& map) const noexcept {
        PipelineCommand command{};
        command.cmd = std::string(cmd);
        command.params.emplace_back(key);
        for (auto& [k, v] : map) {
            command.params.push_back(k);
            command.params.emplace_back(std::move(v));
        }
        {
            std::lock_guard sl(m_mutex);
            m_pipeline_buffer.emplace_back(std::move(command));
        }
    }

    std::future<std::expected<Redis::SomeReplies, RedisError>> Redis::flush_pipeline() const noexcept {
        std::deque<PipelineCommand> commands;
        {
            std::lock_guard sl(m_mutex);
            if (m_pipeline_buffer.empty()) {
                std::promise<std::expected<SomeReplies, RedisError>> empty_promise{};
                auto future = empty_promise.get_future();
                empty_promise.set_value({});
                return future;
            }
            commands.swap(m_pipeline_buffer);
        }

        std::promise<std::expected<SomeReplies, RedisError>> promise{};
        std::future<std::expected<SomeReplies, RedisError>> future = promise.get_future();
        PipelineRequest request{};
        request.commands = std::move(commands);
        request.promise_ptr = std::move(promise);
        {
            std::lock_guard sl(m_mutex);
            m_requests.emplace(std::move(request));
        }
        m_cv.notify_one();
        return future;
    }
}
