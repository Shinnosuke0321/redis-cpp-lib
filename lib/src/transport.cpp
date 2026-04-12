//
// Created by Shinnosuke Kawai on 4/9/26.
//
#include "rediscxx/transport.h"
#include <database/connection_error.h>
#include "rediscxx/internal/arg_parser.h"

namespace rediscxx {
    void transport::run_select_db() noexcept{
        auto select = smart_ptr::make_intrusive<select_command>(
            m_connection_state.db_index,
            [this](std::expected<void, error::redis_exception> res) {
                if (!res) {
                    m_connection_state.handler(std::unexpected(std::move(res.error())));
                    return;
                }
                m_connection_state.handler({});
            });;

        m_exec->post([this, select] {
            select->execute(m_ctx.get());
        });
    }

    void transport::run_auth() noexcept{
        auto cmd = smart_ptr::make_intrusive<auth_command>(
        m_connection_state.password.value(),
        [this](std::expected<void, error::redis_exception> res) {
            if (!res) {
                m_connection_state.handler(std::unexpected(std::move(res.error())));
                return;
            }
            run_select_db();
        });

        m_exec->post([this, cmd] {
            cmd->execute(m_ctx.get());
        });
    }

    void transport::on_connect_failed() noexcept {
        redisAsyncContext* ctx = this->m_ctx.release();
        this->m_connection_state.handler(error::from_ctx(ctx));
        redisAsyncDisconnect(ctx);
    }

    void transport::on_connect(const redisAsyncContext *ctx, const int state) noexcept {
        std::println("on_connect fired");
        const auto self = static_cast<transport*>(ctx->data);
        if (state != REDIS_OK) {
            self->on_connect_failed();
            return;
        }
        if (self->m_connection_state.password) {
            self->run_auth();
        } else {
            self->run_select_db();
        }
    }
    void transport::on_disconnect(const redisAsyncContext *ctx, const int status) noexcept {
        std::println("on_disconnect fired");
    }
    void transport::connect_async(std::string host, const int port, std::optional<std::string> password, const int db_index, on_connected&& callback) noexcept {
        using namespace error;
        using code::EventLoopAttachFailed, code::CallbackRegistrationFailed, code::AsyncConnectFailed;
        m_exec->post([this, host = std::move(host), port, password = std::move(password), db_index, callback = std::move(callback)] {
            redisOptions options = {};
            // 1. TCP connection
            REDIS_OPTIONS_SET_TCP(&options, host.c_str(), port);

            // 2. Timeout
            constexpr timeval tv{1, 50000};
            options.connect_timeout = &tv;

            std::unique_ptr<redisAsyncContext, async_ctx_deleter> ctx(redisAsyncConnectWithOptions(&options));
            if (!ctx) {
                callback(MAKE_UNEXPECTED_ERROR(redis_exception, AsyncConnectFailed, "Failed to connect"));
                return;
            }

            if (ctx->err != REDIS_OK) {
                callback(MAKE_UNEXPECTED_ERROR(redis_exception, AsyncConnectFailed, ctx->errstr));
                return;
            }
            ctx->data = this;
            m_connection_state.password = password;
            m_connection_state.db_index = db_index;
            m_connection_state.handler = callback;
            if (redisLibeventAttach(ctx.get(), m_exec->base()) != REDIS_OK) {
                callback(MAKE_UNEXPECTED_ERROR(redis_exception, EventLoopAttachFailed, "Failed to attach libevent"));
                return;
            }
            if (redisAsyncSetConnectCallback(ctx.get(), on_connect) != REDIS_OK) {
                callback(MAKE_UNEXPECTED_ERROR(redis_exception, CallbackRegistrationFailed, "Failed to set connect callback"));
                return;
            }
            if (redisAsyncSetDisconnectCallback(ctx.get(), on_disconnect) != REDIS_OK) {
                callback(MAKE_UNEXPECTED_ERROR(redis_exception, CallbackRegistrationFailed, "Failed to set disconnect callback"));
                return;
            }
            m_ctx = std::move(ctx);
        });
    }
}
