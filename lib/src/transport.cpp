//
// Created by Shinnosuke Kawai on 4/9/26.
//
#include "rediscxx/transport.h"

#include "core/error/connection_error.h"
#include "rediscxx/internal/arg_parser.h"

namespace rediscxx {
    void transport::run_select_db() {
        auto select = std::make_shared<select_command>(
            m_connection_state.db_index,
            [this](std::expected<void, redis_exception> res) {
                if (!res) {
                    m_connection_state.handler(std::unexpected(std::move(res.error())));
                    return;
                }
                m_connection_state.handler({});
            });;

        m_exec->post([this, select] {
            select->execute(m_ctx);
        });
    }

    void transport::run_auth() {
        auto cmd = std::make_shared<auth_command>(
        m_connection_state.password.value(),
        [this](std::expected<void, redis_exception> res) {
            if (!res) {
                m_connection_state.handler(std::unexpected(std::move(res.error())));
                return;
            }
            run_select_db();
        });

        m_exec->post([this, cmd] {
            cmd->execute(m_ctx);
        });
    }

    void transport::on_connect(const redisAsyncContext *ctx, const int state) noexcept {
        const auto self = static_cast<transport*>(ctx->data);
        if (state != REDIS_OK) {
            self->m_connection_state.handler(std::unexpected(CONNECTION_ERROR(ConnectionFailed, "Failed to connect")));
            return;
        }
        if (self->m_connection_state.password) {
            self->run_auth();
        } else {
            self->run_select_db();
        }
    }
    void transport::on_disconnect(const redisAsyncContext *ctx, const int status) noexcept {

    }
    void transport::connect_async(std::string host, const int port, std::optional<std::string> password, const int db_index, on_connected&& callback) noexcept {
        m_exec->post([this, host = std::move(host), port, password = std::move(password), db_index, callback = std::move(callback)] {
            redisOptions options = {};
            // 1. TCP connection
            REDIS_OPTIONS_SET_TCP(&options, host.c_str(), port);

            // 2. Timeout
            constexpr timeval tv{1, 50000};
            options.connect_timeout = &tv;

            m_ctx = redisAsyncConnectWithOptions(&options);
            if (!m_ctx || m_ctx->err) {
                if (m_ctx && m_ctx->err == REDIS_ERR_IO) {
                }
            }
            m_ctx->data = this;
            m_connection_state.password = password;
            m_connection_state.db_index = db_index;
            m_connection_state.handler = callback;
            if (redisLibeventAttach(m_ctx, m_exec->base()) != REDIS_OK) {
                callback(std::unexpected(CONNECTION_ERROR(ConnectionFailed, "Failed to attach libevent")));
                return;
            }
            if (redisAsyncSetConnectCallback(m_ctx, on_connect) != REDIS_OK) {
                callback(std::unexpected(CONNECTION_ERROR(ConnectionFailed, "Failed to set connect callback")));
                return;
            }
            if (redisAsyncSetDisconnectCallback(m_ctx, on_disconnect) != REDIS_OK) {
                callback(std::unexpected(CONNECTION_ERROR(ConnectionFailed, "Failed to set disconnect callback")));
            }
        });
    }
}
