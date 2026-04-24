//
// Created by Shinnosuke Kawai on 4/8/26.
//
#pragma once
#include <future>
#include <variant>
#include <vector>

#include "event/event_loop_executer.h"
#include "error/exception.h"
#include "internal/arg_buffer.h"
#include "result/reply.h"
#include "rediscxx/command/command_dispatcher.h"

namespace rediscxx {
    struct async_ctx_deleter {
        void operator()(redisAsyncContext* ctx) const noexcept {
            if (ctx) {
                redisAsyncFree(ctx);
            }
        }
    };
    class transport: public core::ref_counted<transport> {
    public:
        using on_connected = std::function<void(std::expected<void, std::variant<event::event_loop_error, error::redis_exception>>)>;
    public:
        explicit transport(smart_ptr::intrusive_ptr<event::event_loop_executer> event_executer)
        : m_exec(std::move(event_executer)) {}

        void connect_async(std::string host, int port, std::optional<std::string> password, int db_index, on_connected&& callback) noexcept;

        void execute_cmd_async(command cmd, internal::arg_buffer&& args, std::function<void(std::expected<result::reply, error::redis_exception>)>&& fn) const;
        redisAsyncContext* get_ctx() const noexcept {
            return m_ctx.get();
        }
        ~transport() override = default;
    private:
        enum class state_type {
            connecting, connected, disconnected, failed,
        };
        using connect_result_t = std::expected<void, std::variant<event::event_loop_error, error::redis_exception>>;
        struct connection_state {
            state_type state = state_type::disconnected;
            std::optional<std::string> password = std::nullopt;
            int db_index = 0;
            std::vector<on_connected> handlers;
        };
        static void on_connect(const redisAsyncContext* ctx, int state) noexcept;
        static void on_disconnect(const redisAsyncContext* ctx, int state) noexcept;
        void on_connect_failed() const noexcept;
        void notify_all(const connect_result_t &result) const noexcept;
        void run_auth() const noexcept;
        void auth_callback(std::expected<result::reply, error::redis_exception> res) const noexcept;
        void select_callback(std::expected<result::reply, error::redis_exception> res) const noexcept;
        void run_select_db() const noexcept;
    private:
        mutable std::mutex m_state_mtx;
        bool m_exec_initialized = false;
        mutable connection_state m_connection_state{};
        smart_ptr::intrusive_ptr<event::event_loop_executer> m_exec;
        std::unique_ptr<redisAsyncContext, async_ctx_deleter> m_ctx;
    };
}
