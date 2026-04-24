//
// Created by Shinnosuke Kawai on 4/9/26.
//
#include "rediscxx/transport.h"

namespace rediscxx {
    void transport::notify_all(const connect_result_t &result) const noexcept {
        std::vector<on_connected> handlers;
        {
            std::lock_guard lock(m_state_mtx);
            m_connection_state.state = result ? state_type::connected : state_type::failed;
            std::swap(handlers, m_connection_state.handlers);
        }
        for (auto& h : handlers)
            h(result);

    }

    void transport::select_callback(std::expected<result::reply, error::redis_exception> res) const noexcept {
        if (!res) {
            notify_all(std::unexpected(connect_result_t::error_type(std::move(res.error()))));
            return;
        }
        notify_all({});
    }

    void transport::run_select_db() const noexcept{
        const command_dispatcher dispatcher(command::select);
        std::string db_index = std::to_string(m_connection_state.db_index);
        auto db_index_arg = internal::arg_buffer::from_list({std::move(db_index)});
        dispatcher.dispatch(
        std::move(db_index_arg),
        [this](std::expected<result::reply, error::redis_exception> res) {
            this->select_callback(std::move(res));
        },
        [this](smart_ptr::intrusive_ptr<command_executor_base> cmd_exe) {
            m_exec->post([this, cmd_exe = std::move(cmd_exe)] { cmd_exe->execute(m_ctx.get());});
        });
    }

    void transport::auth_callback(std::expected<result::reply, error::redis_exception> res) const noexcept {
        if (!res) {
            notify_all(std::unexpected(connect_result_t::error_type(std::move(res.error()))));
            return;
        }
        run_select_db();
    }

    void transport::run_auth() const noexcept {
        const command_dispatcher dispatcher(command::auth);
        auto password_arg = internal::arg_buffer::from_list({m_connection_state.password.value()});
        dispatcher.dispatch(
            std::move(password_arg),
            [this](std::expected<result::reply, error::redis_exception> res) {
                this->auth_callback(std::move(res));
            },
            [this](smart_ptr::intrusive_ptr<command_executor_base> cmd_exe) {
                m_exec->post([this, cmd_exe = std::move(cmd_exe)] { cmd_exe->execute(m_ctx.get());});
        });
    }

    void transport::on_connect_failed() const noexcept {
        auto err = error::from_ctx(m_ctx.get());
        notify_all(std::unexpected(connect_result_t::error_type(std::move(err))));
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
            std::println("on_disconnect fired with status {}", status);
        }
    }
    void transport::connect_async(std::string host, const int port, std::optional<std::string> password, const int db_index, on_connected&& callback) noexcept {
        using namespace error;
        using types::AsyncConnectFailed, types::EventLoopAttachFailed, types::CallbackRegistrationFailed;
        {
            std::unique_lock lock(m_state_mtx);
            switch (m_connection_state.state) {
                case state_type::connected:
                    lock.unlock();
                    callback({});
                    return;
                case state_type::connecting:
                    m_connection_state.handlers.push_back(std::move(callback));
                    return;
                case state_type::failed:
                case state_type::disconnected:
                    break;
            }
            m_connection_state.state = state_type::connecting;
            m_connection_state.handlers.push_back(std::move(callback));

            if (!m_exec_initialized) {
                if (auto res = m_exec->init(); !res) {
                    m_connection_state.state = state_type::failed;
                    std::vector<on_connected> handlers;
                    std::swap(handlers, m_connection_state.handlers);
                    lock.unlock();
                    connect_result_t err = std::unexpected(connect_result_t::error_type(std::move(res.error())));
                    for (auto& h : handlers)
                        h(err);
                    return;
                }
                m_exec_initialized = true;
            }
        }
        auto self = this->intrusive_from_this();
        m_exec->post([self, host = std::move(host), port, password = std::move(password), db_index] {
            redisOptions options = {};
            REDIS_OPTIONS_SET_TCP(&options, host.c_str(), port);

            // disable automatic free replies
            options.options |= REDIS_OPT_NOAUTOFREEREPLIES | REDIS_OPT_NOAUTOFREE;

            constexpr timeval tv{1, 50000};
            options.connect_timeout = &tv;

            std::unique_ptr<redisAsyncContext, async_ctx_deleter> ctx(redisAsyncConnectWithOptions(&options));
            if (!ctx) {
                self->notify_all(MAKE_UNEXPECTED_ERROR(redis_exception, AsyncConnectFailed, "Failed to connect"));
                return;
            }

            if (ctx->err != REDIS_OK) {
                self->notify_all(MAKE_UNEXPECTED_ERROR(redis_exception, AsyncConnectFailed, ctx->errstr));
                return;
            }
            ctx->data = self.get();
            if (redisLibeventAttach(ctx.get(), self->m_exec->base()) != REDIS_OK) {
                self->notify_all(MAKE_UNEXPECTED_ERROR(redis_exception, EventLoopAttachFailed, "Failed to attach libevent"));
                return;
            }
            if (redisAsyncSetConnectCallback(ctx.get(), on_connect) != REDIS_OK) {
                self->notify_all(MAKE_UNEXPECTED_ERROR(redis_exception, CallbackRegistrationFailed, "Failed to set connect callback"));
                return;
            }
            if (redisAsyncSetDisconnectCallback(ctx.get(), on_disconnect) != REDIS_OK) {
                self->notify_all(MAKE_UNEXPECTED_ERROR(redis_exception, CallbackRegistrationFailed, "Failed to set disconnect callback"));
                return;
            }
            self->m_ctx = std::move(ctx);
            self->m_connection_state.password = password;
            self->m_connection_state.db_index = db_index;
        });
    }

    void transport::execute_cmd_async(const command cmd, internal::arg_buffer &&args, std::function<void(std::expected<result::reply, error::redis_exception>)> &&fn) const {
        const command_dispatcher dispatcher(cmd);
        dispatcher.dispatch(std::move(args), std::move(fn),
            [this](smart_ptr::intrusive_ptr<command_executor_base> cmd_ex) {
                m_exec->post([this, cmd_ex = std::move(cmd_ex)] { cmd_ex->execute(m_ctx.get());});
        });
    }
}