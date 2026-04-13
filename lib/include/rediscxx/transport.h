//
// Created by Shinnosuke Kawai on 4/8/26.
//
#pragma once
#include <future>
#include "event/event_loop_executer.h"
#include "command/auth_command.h"
#include "command/select_command.h"
#include "error/exception.h"
#include "internal/arg_buffer.h"
#include "result/reply.h"
#include "rediscxx/command/command_creater.h"

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
        using on_connected = std::function<void(std::expected<void, error::redis_exception>)>;
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
        struct connection_state {
            std::optional<std::string> password = std::nullopt;
            int db_index = 0;
            on_connected handler = nullptr;
        };
        static void on_connect(const redisAsyncContext* ctx, int state) noexcept;
        static void on_disconnect(const redisAsyncContext* ctx, int state) noexcept;
        void on_connect_failed() noexcept;
        void run_auth() noexcept;
        void run_select_db() noexcept;
    private:
        smart_ptr::intrusive_ptr<event::event_loop_executer> m_exec;
        std::unique_ptr<redisAsyncContext, async_ctx_deleter> m_ctx;
        connection_state m_connection_state{};
    };
}
