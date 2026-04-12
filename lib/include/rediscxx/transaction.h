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

        void discard() noexcept {
            m_watch_keys.clear();
        }

    public:
        transaction(const transaction&) = delete;
        transaction& operator=(const transaction&) = delete;
        transaction(transaction&&) = delete;
        transaction& operator=(transaction&&) = delete;
        ~transaction() = default;

    private:

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
    };
}
