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
#include "result/reply.h"
#include "transaction.h"
#include <core/memory/intrusive_ptr.h>
#include "transport.h"
#include "event/event_loop_executer.h"
#include "internal/arg_parser.h"

namespace rediscxx {
    class client final : public database::IConnection, public core::ref_counted<client> {
    public:
        using on_success = std::function<void(result::reply)>;
        using on_error = std::function<void(error::redis_exception)>;
        struct config {
            std::string host;
            int port;
            std::optional<std::string> password;
            int dbIndex = 0;
        };
    public:
        explicit client(const config& config)
        : m_config(config),
          m_event_executer(smart_ptr::make_intrusive<event::event_loop_executer>()),
          m_transport(smart_ptr::make_intrusive<transport>(m_event_executer->intrusive_from_this())) {}

        explicit client(config&& config)
        : m_config(std::move(config)),
          m_event_executer(smart_ptr::make_intrusive<event::event_loop_executer>()),
          m_transport(smart_ptr::make_intrusive<transport>(m_event_executer->intrusive_from_this())) {}

        client(const client&) = delete;
        client& operator=(const client&) = delete;
        client(client&&) noexcept = delete;
        client& operator=(client&&) noexcept = delete;
        ~client() override;

    public:
        std::expected<void, core::error::exception> connect() const noexcept;

        template<typename... Params>
        requires (internal::is_supported_type_v<Params> && ...)
        std::future<std::expected<result::reply, error::redis_exception>> execute_command(const command cmd, Params&&... params) noexcept {
            auto promise = std::make_shared<std::promise<std::expected<result::reply, error::redis_exception>>>();
            auto fut = promise->get_future();
            internal::arg_buffer args = internal::to_args(std::forward<Params>(params)...);
            m_transport->execute_cmd_async(cmd, std::move(args), [promise](std::expected<result::reply, error::redis_exception> expected_res) {
                promise->set_value(std::move(expected_res));
            });
            return fut;
        }

        template<typename... Params>
        requires (internal::is_supported_type_v<Params> && ...)
        void execute_command(on_success&& on_success, on_error&& on_error, const command cmd, Params&&... params) noexcept {
            auto promise = std::make_shared<std::promise<std::expected<result::reply, error::redis_exception>>>();
            auto fut = promise->get_future();
            internal::arg_buffer args = internal::to_args(std::forward<Params>(params)...);
            m_transport->execute_cmd_async(
                cmd, std::move(args),
                [on_success = std::move(on_success), on_error = std::move(on_error)](std::expected<result::reply, error::redis_exception> expected_res) {
                    if (expected_res) {
                        on_success(std::move(expected_res.value()));
                    } else {
                        on_error(std::move(expected_res.error()));
                    }
            });
        }


    private:
        config m_config;
        smart_ptr::intrusive_ptr<event::event_loop_executer> m_event_executer;
        smart_ptr::intrusive_ptr<transport> m_transport;
    };

}
