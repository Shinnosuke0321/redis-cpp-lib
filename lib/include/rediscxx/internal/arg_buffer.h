//
// Created by Shinnosuke Kawai on 4/4/26.
//

#pragma once
#include <string>

namespace rediscxx::internal {
    class arg_buffer {
    public:
        static arg_buffer from_list(std::vector<std::string>&& args) noexcept;
        [[nodiscard]]
        int argc() const noexcept { return static_cast<int>(m_lens.size()); }
        [[nodiscard]]
        std::vector<const char*> argv() const noexcept;
        [[nodiscard]]
        const std::vector<size_t>& argvlen() const noexcept { return m_lens; }
    public:
        arg_buffer(arg_buffer&& other) noexcept
        : m_storage(std::move(other.m_storage)),
          m_lens(std::move(other.m_lens)) {}
        arg_buffer& operator=(arg_buffer&& other) noexcept {
            if (this != &other) {
                m_lens = std::move(other.m_lens);
                m_storage = std::move(other.m_storage);
            }
            return *this;
        }
        arg_buffer(const arg_buffer&) = delete;
        arg_buffer& operator=(const arg_buffer&) = delete;
    private:
        arg_buffer() = default;
        std::string m_storage;
        std::vector<std::size_t> m_lens;
    };
}