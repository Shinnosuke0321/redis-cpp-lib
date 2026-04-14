//
// Created by Shinnosuke Kawai on 4/9/26.
//
#include "rediscxx/transport.h"

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
        command_dispatcher dispatcher(cmd, std::move(args), std::move(fn));
        dispatcher.dispatch<set_command>([this](smart_ptr::intrusive_ptr<set_command> set_cmd) {
            m_exec->post([this, set_cmd = std::move(set_cmd)] { set_cmd->execute(m_ctx.get());});
        });
        dispatcher.dispatch<get_command>([this](smart_ptr::intrusive_ptr<get_command> get_cmd) {
            m_exec->post([this, get_cmd = std::move(get_cmd)] { get_cmd->execute(m_ctx.get());});
        });
        dispatcher.dispatch<hset_command>([this](smart_ptr::intrusive_ptr<hset_command> hset_cmd) {
            m_exec->post([this, hset_cmd = std::move(hset_cmd)] { hset_cmd->execute(m_ctx.get());});
        });
        dispatcher.dispatch<flush_db_command>([this](smart_ptr::intrusive_ptr<flush_db_command> flush_cmd) {
            m_exec->post([this, flush_cmd = std::move(flush_cmd)] { flush_cmd->execute(m_ctx.get());});
        });
        dispatcher.dispatch<hget_command>([this](smart_ptr::intrusive_ptr<hget_command> hget_cmd) {
            m_exec->post([this, hget_cmd = std::move(hget_cmd)] { hget_cmd->execute(m_ctx.get());});
        });
        dispatcher.dispatch<hgetall_command>([this](smart_ptr::intrusive_ptr<hgetall_command> hgetall_cmd) {
            m_exec->post([this, hgetall_cmd = std::move(hgetall_cmd)] { hgetall_cmd->execute(m_ctx.get());});
        });
        dispatcher.dispatch<sadd_command>([this](smart_ptr::intrusive_ptr<sadd_command> sadd_cmd) {
            m_exec->post([this, sadd_cmd = std::move(sadd_cmd)] { sadd_cmd->execute(m_ctx.get());});
        });
        dispatcher.dispatch<srem_command>([this](smart_ptr::intrusive_ptr<srem_command> srem_cmd) {
            m_exec->post([this, srem_cmd = std::move(srem_cmd)] { srem_cmd->execute(m_ctx.get());});
        });
        dispatcher.dispatch<smembers_command>([this](smart_ptr::intrusive_ptr<smembers_command> smembers_cmd) {
            m_exec->post([this, smembers_cmd = std::move(smembers_cmd)] { smembers_cmd->execute(m_ctx.get());});
        });
    }
}
