//
// Created by Shinnosuke Kawai on 3/28/26.
//
#include "rediscxx/result/reply.h"

namespace rediscxx::result {

    reply reply::from_raw(const redisReply* r) noexcept {
        reply out;
        out.m_type    = r->type;
        out.m_integer = r->integer;
        out.m_dval    = r->dval;

        if (r->str && r->len > 0)
            out.m_str.assign(r->str, r->len);

        if (r->elements > 0 && r->element) {
            out.m_elements.reserve(r->elements);
            for (size_t i = 0; i < r->elements; ++i) {
                const redisReply* e = r->element[i];
                element el;
                if (e) {
                    el.type    = e->type;
                    el.integer = e->integer;
                    el.dval    = e->dval;
                    if (e->str && e->len > 0)
                        el.str.assign(e->str, e->len);
                }
                out.m_elements.push_back(std::move(el));
            }
        }
        return out;
    }

    bool reply::is_okay() const noexcept {
        return m_type == REDIS_REPLY_STATUS && !m_str.empty() && m_str[0] == '+';
    }

    bool reply::is_nil() const noexcept {
        return m_type == REDIS_REPLY_NIL;
    }

    std::optional<std::deque<reply>> reply::as_replies() const noexcept {
        if (m_type != REDIS_REPLY_ARRAY)
            return std::nullopt;
        std::deque<reply> result;
        for (const auto&[type, str, integer, dval] : m_elements) {
            reply r;
            r.m_type    = type;
            r.m_str     = str;
            r.m_integer = integer;
            r.m_dval    = dval;
            result.push_back(std::move(r));
        }
        return std::move(result);
    }

    std::optional<std::string> reply::as_string() const noexcept {
        if (m_type == REDIS_REPLY_STRING || m_type == REDIS_REPLY_STATUS || m_type == REDIS_REPLY_VERB)
            return m_str;
        return std::nullopt;
    }

    std::optional<int64_t> reply::as_integer() const noexcept {
        if (m_type == REDIS_REPLY_INTEGER)
            return (m_integer);
        return std::nullopt;
    }

    std::optional<double> reply::as_double() const noexcept {
        if (m_type == REDIS_REPLY_DOUBLE)
            return m_dval;
        return std::nullopt;
    }

    std::optional<std::vector<std::string>> reply::as_array() const noexcept {
        if (m_type != REDIS_REPLY_ARRAY)
            return std::nullopt;
        std::vector<std::string> result(m_elements.size());
        for (const auto& e : m_elements) {
            if (e.type == REDIS_REPLY_STRING || e.type == REDIS_REPLY_STATUS)
                result.push_back(e.str);
        }
        return std::move(result);
    }

    std::optional<std::unordered_map<std::string, std::string>> reply::as_map() const noexcept {
        if (m_type != REDIS_REPLY_ARRAY)
            return std::nullopt;
        if (m_elements.size() % 2 != 0)
            return std::nullopt;
        std::unordered_map<std::string, std::string> result;
        result.reserve(m_elements.size() / 2);
        for (size_t i = 0; i < m_elements.size(); i += 2)
            result.emplace(m_elements[i].str, m_elements[i + 1].str);
        return std::move(result);
    }

    std::optional<std::unordered_set<std::string>> reply::as_set() const noexcept {
        if (m_type != REDIS_REPLY_ARRAY)
            return std::nullopt;
        std::unordered_set<std::string> result;
        result.reserve(m_elements.size());
        for (const auto& e : m_elements)
            result.insert(e.str);
        return std::move(result);
    }
}
