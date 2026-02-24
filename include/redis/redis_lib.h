//
// Created by Shinnosuke Kawai on 10/21/25.
//
// RedisConnection.h
#pragma once
#include <future>
#include <optional>
#include <deque>
#include <variant>
#include <string>
#include <database/connection.h>
#include <hiredis.h>
#include <queue>
#include <unordered_set>
#include "redis_error.h"
#include <functional>
#include <thread>

namespace Database {

    struct RedisContextDeleter {
        void operator()(redisContext* ctx) const noexcept {
            if (ctx) {
                redisFree(ctx);
            }
        }
    };

    struct RedisReplyDeleter {
        void operator()(redisReply* reply) const noexcept {
            if (reply) {
                freeReplyObject(reply);
            }
        }
    };
    class Redis final : public Core::Database::IConnection {
    public:
        using UniqueContext = std::unique_ptr<redisContext, RedisContextDeleter>;
        using UniqueReply = std::unique_ptr<redisReply, RedisReplyDeleter>;
        using ResultCallback = std::function<void(UniqueReply)>;
        using ErrorCallback = std::function<void(const RedisError &)>;
        using Errors = std::deque<RedisError>;
        using SomeReplies = std::deque<UniqueReply>;

    public:
        struct ConnectionConfig {
            std::string host;
            int port;
            std::optional<std::string> password;
            int dbIndex = 0;
        };
        struct PipelineCommand {
            std::string cmd{};
            std::deque<std::string> params{};
        };
    public:
        static std::expected<std::unique_ptr<Redis>, Core::Database::ConnectionError> ConnectionFactory() noexcept;

        explicit Redis(ConnectionConfig&& config): m_config(std::move(config)) {}
        Redis(const Redis&) = delete;
        Redis& operator=(const Redis&) = delete;
        Redis(Redis&&) noexcept = delete;
        Redis& operator=(Redis&&) noexcept = delete;
        ~Redis() override;

    public:
        void execute_command_async(std::string_view cmd, std::string_view key, ResultCallback&& callback, ErrorCallback&& err_callback) const noexcept;
        void execute_command_async(std::string_view cmd, std::string_view key, std::string_view str_val, ResultCallback&& callback, ErrorCallback&& err_callback) const noexcept;
        void execute_command_async(std::string_view cmd, std::string_view key, std::unordered_map<std::string, std::string>&& map, ResultCallback&& callback, ErrorCallback&& err_callback) const noexcept;
        void execute_command_async(std::string_view cmd, std::string_view key, std::unordered_set<std::string>&& set, ResultCallback&& callback, ErrorCallback&& err_callback) const noexcept;

        std::future<std::expected<UniqueReply, RedisError>> execute_command(std::string_view cmd, std::string_view key) const noexcept;
        std::future<std::expected<UniqueReply, RedisError>> execute_command(std::string_view cmd, std::string_view key, std::string_view str_val) const noexcept;
        std::future<std::expected<UniqueReply, RedisError>> execute_command(std::string_view cmd, std::string_view key, std::unordered_map<std::string, std::string>&& map) const noexcept;
        std::future<std::expected<UniqueReply, RedisError>> execute_command(std::string_view cmd, std::string_view key, std::unordered_set<std::string>&& set) const noexcept;

        void append(std::string_view cmd, std::string_view key) const noexcept;
        void append(std::string_view cmd, std::string_view key, std::string_view str_val) const noexcept;
        void append(std::string_view cmd, std::string_view key, std::deque<std::string>&& params) const noexcept;
        void append(std::string_view cmd, std::string_view key, std::unordered_map<std::string, std::string>&& map) const noexcept;
        void append(std::string_view cmd, std::string_view key, std::unordered_set<std::string>&& set) const noexcept;
        std::future<std::expected<SomeReplies, RedisError>> flush_pipeline() const noexcept;

    private:
        struct RedisRequest {
            std::string cmd{};
            std::deque<std::string> params{};
            ResultCallback on_success;
            ErrorCallback on_error;
        };
        struct PipelineRequest {
            std::deque<PipelineCommand> commands{};
            std::promise<std::expected<SomeReplies, RedisError>> promise_ptr;
        };
        using QueueItem = std::variant<RedisRequest, PipelineRequest>;
    private:
        std::expected<void, Core::Database::ConnectionError> connect() const noexcept;
        void worker_thread_func(const std::stop_token &st) const noexcept;

    private:
        ConnectionConfig m_config;
        mutable UniqueContext m_context = nullptr;
        mutable std::mutex m_mutex;
        mutable std::condition_variable m_cv;
        mutable std::queue<QueueItem> m_requests;
        mutable std::deque<PipelineCommand> m_pipeline_buffer;
        mutable std::jthread m_worker_thread;
    };

}
