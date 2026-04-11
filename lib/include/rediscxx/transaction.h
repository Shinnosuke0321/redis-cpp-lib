//
// Created by Shinnosuke Kawai on 3/28/26.
//
#pragma once
#include <deque>
#include "error/exception.h"
#include <future>
#include <expected>
#include <optional>
#include <vector>
#include <string>
#include "command/command.h"

namespace rediscxx {
    class client;

    class transaction {
    public:
        void watch(std::string key) noexcept {
            m_watch_keys.push_back(std::move(key));
        }

        template<typename ...Params>
        requires (internal::is_supported_type_v<Params> && ...)
        void enqueue(const command cmd, Params&& ...params) noexcept {
            internal::arg_buffer buf = internal::to_args(command_to_string(cmd), std::forward<Params>(params)...);
            m_queued_commands.emplace_back(std::move(buf));
        }

        void discard() noexcept {
            m_watch_keys.clear();
            m_queued_commands.clear();
        }

    public:
        transaction(const transaction&) = delete;
        transaction& operator=(const transaction&) = delete;
        transaction(transaction&&) = delete;
        transaction& operator=(transaction&&) = delete;
        ~transaction() = default;

    private:
        std::future<std::expected<std::optional<std::deque<result::reply>>, exception>> exec() noexcept;

        void exec(std::function<void(std::optional<std::deque<result::reply>>)>&& success_cb,
                  std::function<void(exception)>&& error_cb) noexcept;

    private:
        struct passkey {
            friend class client;
        private:
            explicit passkey() = default;
        };
        explicit transaction(const client* owner, passkey) noexcept : m_client(owner) {}

    private:
        friend class client;
        const client* m_client;
        std::vector<std::string> m_watch_keys{};
        std::deque<single_request> m_queued_commands{};
    };
}
