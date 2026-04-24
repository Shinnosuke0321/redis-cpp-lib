//
// Created by Shinnosuke Kawai on 3/28/26.
//
#pragma once
#include <deque>
#include <optional>
#include <string>
#include <hiredis/hiredis.h>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace rediscxx::result {
    class reply {
    public:
        using array = std::vector<std::string>;
        using map = std::unordered_map<std::string, std::string>;
        using set = std::unordered_set<std::string>;
    public:
        struct element {
            int type = REDIS_REPLY_NIL;
            std::string str;
            long long integer = 0;
            double dval = 0.0;
        };

        // Deep-copy a redisReply without taking ownership.
        // Use this inside hiredis async callbacks where hiredis owns the reply.
        static reply from_raw(redisReply* r) noexcept;

        bool is_okay() const noexcept;
        bool is_nil() const noexcept;
        std::optional<std::string> as_string() const noexcept;
        std::optional<int64_t> as_integer() const noexcept;
        std::optional<std::vector<std::string>> as_array() const noexcept;
        std::optional<std::unordered_map<std::string, std::string>> as_map() const noexcept;
        std::optional<std::unordered_set<std::string>> as_set() const noexcept;
        std::optional<double> as_double() const noexcept;
        std::optional<std::deque<reply>> as_replies() const noexcept;

        reply(reply&&) noexcept = default;
        reply& operator=(reply&&) noexcept = default;
        reply(const reply&) = delete;
        reply& operator=(const reply&) = delete;
        ~reply() noexcept = default;

    private:
        reply() = default;

        int m_type = REDIS_REPLY_NIL;
        std::string m_str;
        long long m_integer = 0;
        double m_dval = 0.0;
        std::vector<element> m_elements;
    };
}
