//
// Created by Shinnosuke Kawai on 10/21/25.
//
// RedisConnection.h
#pragma once
#include <deque>
#include <future>
#include <optional>
#include <string>
#include <database/connection.h>
#include <event2/thread.h>
#include <hiredis/adapters/libevent.h>
#include <hiredis/async.h>
#include "request.h"
#include "result/reply.h"
#include "transaction.h"
#include <core/memory/intrusive_ptr.h>
#include "transport.h"
#include "event/event_loop_executer.h"

namespace rediscxx {

    class pipeline_event;

    struct context_deleter {
        void operator()(redisAsyncContext* ctx) const {
            if (ctx) {
                redisAsyncFree(ctx);
            }
        }
    };

    struct ev_loop_deleter {
        void operator()(event_base* ev) const {
            if (ev) {
                event_base_free(ev);
            }
        }
    };

    class client final : public Core::Database::IConnection, public core::ref_counted<client> {
    public:
        struct config {
            std::string host;
            int port;
            std::optional<std::string> password;
            int dbIndex = 0;
        };
    public:
        explicit client(const config& config)
        : m_config(config),
          m_transport(smart_ptr::make_intrusive<transport>(m_event_executer->intrusive_from_this())) {}

        explicit client(config&& config)
        : m_config(std::move(config)),
          m_transport(smart_ptr::make_intrusive<transport>(m_event_executer->intrusive_from_this())) {}

        client(const client&) = delete;
        client& operator=(const client&) = delete;
        client(client&&) noexcept = delete;
        client& operator=(client&&) noexcept = delete;
        ~client() override;

    public:
        std::expected<void, Core::Database::ConnectionError> connect() const noexcept;

        template<typename... Params>
        requires (internal::is_supported_type_v<Params> && ...)
        std::future<std::expected<result::reply, command_error>> execute_command(const command cmd, Params&&... params) noexcept {
        }

        template<typename... Params>
        requires (internal::is_supported_type_v<Params> && ...)
        void execute_command_async(result_callback&& callback, error_callback&& err_callback, const command cmd, Params&&... params) const noexcept {
        }

        template<typename... Params>
        requires (internal::is_supported_type_v<Params> && ...)
        void append(const command cmd, Params&& ...params) noexcept {
        }

        std::future<std::expected<std::vector<result::reply>, pipeline_error>> execute_pipeline() noexcept;
        std::future<std::expected<std::optional<std::deque<result::reply>>, transaction_error>> perform_transaction(std::function<void(transaction&)>&& executor) const noexcept;

        void perform_transaction_async(
            std::function<void(transaction&)>&& executor,
            std::function<void(std::optional<std::deque<result::reply>>)>&& success_cb= nullptr,
            std::function<void(transaction_error)>&& error_cb= nullptr) const noexcept;

        friend class pipeline_event;
        friend class transaction;

    private:
        static void ConnectCallback(const redisAsyncContext* ctx, int status);
        static void AuthInit(const redisAsyncContext* ctx) noexcept;
        static void SelectDbIndex(const redisAsyncContext* ac) noexcept;
        void Handle(single_request &request) const noexcept;

    private:
        config m_config;
        smart_ptr::intrusive_ptr<event::event_loop_executer> m_event_executer;
        smart_ptr::intrusive_ptr<transport> m_transport;
    };

}
