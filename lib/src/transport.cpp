//
// Created by Shinnosuke Kawai on 4/9/26.
//
#include "rediscxx/transport.h"

namespace rediscxx {
    void transport::run_select_db() noexcept{
        command_dispatcher dispatcher(command::select);
        std::string db_index = std::to_string(m_connection_state.db_index);
        auto db_index_arg = internal::arg_buffer::from_list({std::move(db_index)});
        dispatcher.dispatch(
            std::move(db_index_arg),
            [this](std::expected<result::reply, error::redis_exception> res) {
                if (!res) {
                    m_connection_state.handler(std::unexpected(std::move(res.error())));
                    return;
                }
                m_connection_state.handler({});
        },
        [this](auto cmd_exe) {
            m_exec->post([this, cmd_exe = std::move(cmd_exe)] { cmd_exe->execute(m_ctx.get());});
        });
    }

    void transport::run_auth() noexcept{
        command_dispatcher dispatcher(command::auth);
        auto password_arg = internal::arg_buffer::from_list({m_connection_state.password.value()});
        dispatcher.dispatch(
            std::move(password_arg),
            [this](std::expected<result::reply, error::redis_exception> res) {
                if (!res) {
                    m_connection_state.handler(std::unexpected(std::move(res.error())));
                    return;
                }
                run_select_db();
            },
            [this](auto cmd_exe) {
                m_exec->post([this, cmd_exe = std::move(cmd_exe)] { cmd_exe->execute(m_ctx.get());});
            });
    }

    void transport::on_connect_failed() noexcept {
        redisAsyncContext* ctx = this->m_ctx.release();
        this->m_connection_state.handler(std::unexpected(error::from_ctx(ctx)));
        redisAsyncDisconnect(ctx);
        redisAsyncFree(ctx);
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
        if (status != REDIS_OK) {

        }
    }
    void transport::connect_async(std::string host, const int port, std::optional<std::string> password, const int db_index, on_connected&& callback) noexcept {
        using namespace error;
        using types::EventLoopAttachFailed, types::CallbackRegistrationFailed, types::AsyncConnectFailed;
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

    void transport::execute_cmd_async(const command cmd, internal::arg_buffer &&args, std::function<void(std::expected<result::reply, error::redis_exception>)> &&fn) const {
        command_dispatcher dispatcher(cmd);
        dispatcher.dispatch(std::move(args), std::move(fn),
            [this](auto cmd_ex) {
                m_exec->post([this, cmd_ex = std::move(cmd_ex)] { cmd_ex->execute(m_ctx.get());});
        });
    }
}
