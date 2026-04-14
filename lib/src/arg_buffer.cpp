//
// Created by Shinnosuke Kawai on 4/4/26.
//
#include "rediscxx/internal/arg_parser.h"

namespace rediscxx::internal {
    arg_buffer arg_buffer::from_list(std::vector<std::string> &&args) noexcept {
        std::size_t total = 0;
        arg_buffer result;
        for (const auto& s : args)
            total += s.size();
        result.m_storage.reserve(total);

        for (const auto& s : args) {
            result.m_storage += s;
            result.m_lens.push_back(s.size());
        }
        return result;
    }

    std::vector<const char *> arg_buffer::argv() const noexcept {
        std::vector<const char*> argv;
        argv.reserve(m_lens.size());
        size_t offset = 0;
        for (const auto& len : m_lens) {
            std::string_view cmd{m_storage.c_str() + offset, len};
            argv.push_back(cmd.data());
            offset += len;
        }
        return argv;
    }
    void arg_buffer::append_at_front(std::string_view str) noexcept {
        m_lens.push_front(str.size());
        m_storage.insert(m_storage.begin(), str.begin(), str.end());
    }
}
