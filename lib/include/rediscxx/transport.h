//
// Created by Shinnosuke Kawai on 4/8/26.
//
#pragma once
#include <future>
#include "event/event_loop_executer.h"
#include "command/command.h"
#include "core/error/connection_error.h"

namespace rediscxx {
    class transport: public core::ref_counted<transport> {
    public:
        using on_connected = std::function<void(std::expected<void, core::connection_error>)>;
    public:
        explicit transport(smart_ptr::intrusive_ptr<event::event_loop_executer> event_executer)
        : m_exec(std::move(event_executer)) {}

        void connect_async(std::string host, int port, std::optional<std::string> password, int db_index, on_connected&& callback) noexcept;

        redisAsyncContext* get_ctx() const noexcept {
            return m_ctx;
        }
        ~transport() override {
            if (m_ctx) {
                redisAsyncDisconnect(m_ctx);
            }
        }
    private:
        struct connection_state {
            std::optional<std::string> password = std::nullopt;
            int db_index = 0;
            on_connected handler = nullptr;
        };
        static void on_connect(const redisAsyncContext* ctx, int state) noexcept;
        static void on_disconnect(const redisAsyncContext* ctx, int state) noexcept;
        void run_auth();
        void run_select_db();
    private:
        smart_ptr::intrusive_ptr<event::event_loop_executer> m_exec;
        redisAsyncContext* m_ctx = nullptr;
        connection_state m_connection_state{};
    };
}
