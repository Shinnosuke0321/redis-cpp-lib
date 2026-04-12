//
// Created by Shinnosuke Kawai on 4/8/26.
//

#pragma once
#include <functional>
#include <event2/thread.h>
#include <hiredis/adapters/libevent.h>
#include <queue>
#include <expected>
#include <thread>
#include <mutex>
#include <future>
#include <core/memory/intrusive_ptr.h>
#include "core/error/base_error.h"

namespace rediscxx::event {
    struct executer_base {
        virtual ~executer_base() = default;
        virtual void post(std::function<void()>&& fn) = 0;
    };

    enum class ev_err_types {
        BaseInitFailed, EventInitFailed, EventRegistrationFailed,
    };

    class event_loop_error: public core::error::typed_error<event_loop_error, ev_err_types> {
    public:
        ERROR_CLASS_CATEGORY(event_loop)
        ~event_loop_error() override = default;
    };

    class event_loop_executer: public executer_base, public core::ref_counted<event_loop_executer> {
    public:
        event_loop_executer() = default;

        std::expected<void, event_loop_error> init() noexcept {
            static std::once_flag evthread_init;
            std::call_once(evthread_init, [] {
                if (evthread_use_pthreads() < 0)
                    std::println("Redis: Failed to initialize libevent");
                else
                    std::println("Redis: Initialized libevent");
            });
            m_base = event_base_new();
            if (!m_base)
                RETURN_UNEXPECTED_ERROR(event_loop_error, ev_err_types::BaseInitFailed, "Failed to create event base");

            m_wakeup_event = event_new(m_base, -1, EV_PERSIST,
                [](evutil_socket_t, short, void* arg) {
                    static_cast<event_loop_executer*>(arg)->drain();
                },
                this
            );

            if (!m_wakeup_event) {
                RETURN_UNEXPECTED_ERROR(event_loop_error, ev_err_types::EventInitFailed, "Failed to create wakeup event");
            }

            if (event_add(m_wakeup_event, nullptr) != 0) {
                RETURN_UNEXPECTED_ERROR(event_loop_error, ev_err_types::EventRegistrationFailed, "Failed to register wakeup event");
            }

            std::atomic_bool ready{false};
            m_worker_thread = std::jthread([this, &ready](const std::stop_token&) {
                m_thread_id = std::this_thread::get_id();
                ready.store(true, std::memory_order_release);
                event_base_loop(m_base, EVLOOP_NO_EXIT_ON_EMPTY);
            });
            while (!ready.load(std::memory_order_acquire)) {
                std::this_thread::yield();
            }

            return {};
        }

        ~event_loop_executer() override {
            stop();
            if (m_wakeup_event)
                event_free(m_wakeup_event);
            if (m_base)
                event_base_free(m_base);
        }

        void post(std::function<void()>&& fn) override {
            std::println("post()");
            if (std::this_thread::get_id() == m_thread_id) {
                std::println("already on event thread");
                fn(); // already on event thread
                return;
            }
            {
                std::println("Locking m_tasks");
                std::lock_guard lk(m_mtx);
                m_tasks.push(std::move(fn));
            }
            event_active(m_wakeup_event, 0, 0);
        }

        event_base* base() const { return m_base; }

    private:
        void drain() {
            std::println("drain()");
            std::queue<std::function<void()>> local;
            {
                std::lock_guard lk(m_mtx);
                std::swap(local, m_tasks);
            }
            while (!local.empty()) {
                local.front()();
                local.pop();
            }
        }

        void stop() {
            if (m_stopping.exchange(true))
                return;
            event_base_loopbreak(m_base);
            event_active(m_wakeup_event, 0, 0);

            if (m_worker_thread.joinable())
                m_worker_thread.join();
        }

    private:
        event_base* m_base = nullptr;
        struct event* m_wakeup_event = nullptr;
        std::queue<std::function<void()>> m_tasks{};
        std::atomic<bool> m_stopping{false};
        std::mutex m_mtx;
        std::thread::id m_thread_id;
        std::jthread m_worker_thread;
    };
}
