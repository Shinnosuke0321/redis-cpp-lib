//
// Created by Shinnosuke Kawai on 3/28/26.
//
#pragma once
#include <array>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "arg_buffer.h"
#include "rediscxx/command/base_command.h"

namespace rediscxx::internal {

    // --- Compile-time type support traits ---
    template<typename T>
    struct is_supported_type : std::bool_constant<std::is_arithmetic_v<T>> {};

    template<> struct is_supported_type<command>          : std::true_type {};
    template<> struct is_supported_type<std::string>      : std::true_type {};
    template<> struct is_supported_type<std::string_view> : std::true_type {};
    template<> struct is_supported_type<const char*>      : std::true_type {};
    template<> struct is_supported_type<char*>            : std::true_type {};

    template<typename T>
    struct is_supported_type<std::vector<T>>              : std::true_type {};

    template<typename T, std::size_t N>
    struct is_supported_type<std::array<T, N>>             : std::true_type {};

    template<typename V>
    struct is_supported_type<std::unordered_map<std::string, V>> : std::true_type {};

    template<typename T>
    struct is_supported_type<std::unordered_set<T>>      : std::true_type {};

    template<typename T>
    inline constexpr bool is_supported_type_v = is_supported_type<std::decay_t<T>>::value;

    // --- String conversion helpers (collect into a temp vector) ---

    inline void append_as_strings(std::vector<std::string>& out, const command val) {
        out.emplace_back(command_to_str(val));
    }

    inline void append_as_strings(std::vector<std::string>& out, const std::string& val) {
        out.push_back(val);
    }

    inline void append_as_strings(std::vector<std::string>& out, std::string_view val) {
        out.emplace_back(val);
    }

    inline void append_as_strings(std::vector<std::string>& out, const char* val) {
        out.emplace_back(val);
    }

    inline void append_as_strings(std::vector<std::string>& out, char* val) {
        out.emplace_back(val);
    }

    template<typename T>
    requires std::is_arithmetic_v<std::decay_t<T>>
    void append_as_strings(std::vector<std::string>& out, T val) {
        out.push_back(std::to_string(val));
    }

    template<typename T>
    void append_as_strings(std::vector<std::string>& out, const std::vector<T>& val) {
        for (const auto& elem : val)
            append_as_strings(out, elem);
    }

    template<typename T, std::size_t N>
    void append_as_strings(std::vector<std::string>& out, const std::array<T, N>& val) {
        for (const auto& elem : val)
            append_as_strings(out, elem);
    }

    template<typename V>
    void append_as_strings(std::vector<std::string>& out, const std::unordered_map<std::string, V>& val) {
        for (const auto& [key, value] : val) {
            out.push_back(key);
            append_as_strings(out, value);
        }
    }

    template<typename T>
    void append_as_strings(std::vector<std::string>& out, const std::unordered_set<T>& val) {
        for (const auto& elem : val)
            append_as_strings(out, elem);
    }

    template<typename... Args>
    requires (is_supported_type_v<Args> && ...)
    arg_buffer to_args(Args&&... args) {
        std::vector<std::string> temp;
        (append_as_strings(temp, std::forward<Args>(args)), ...);
        return arg_buffer::from_list(std::move(temp));
    }

}
