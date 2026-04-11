//
// Created by Shinnosuke Kawai on 3/11/26.
//
#pragma once
#include <string>
#include <format>
#include <string_view>
#include <concepts>
#include "exception.h"

namespace core::error {
    class error_base {
    public:
        virtual ~error_base() = default;

        virtual std::string_view message() const noexcept = 0;
        virtual std::string_view category() const noexcept = 0;
        virtual std::string_view code_string() const noexcept = 0;
        virtual std::string to_string() const noexcept = 0;
    };

    template<typename derived>
    concept typed_error_trait = requires {
        { derived::category_name() } -> std::same_as<std::string_view>;
    };

    template<typename derived, typename enum_type>
    class typed_error : public error_base {
        static void _check() {
            static_assert(typed_error_trait<derived>,
            "Derived must implement:\n"
            "  static std::string_view category_name()");
        }
    public:
        using code_type = enum_type;

        typed_error(enum_type code, std::string code_str, std::string msg)
        : m_code(code),
          m_code_str(std::move(code_str)),
          m_message(std::move(msg)) {
            _check();
        }

        std::string_view message() const noexcept override {
            return m_message;
        }

        enum_type get_code() const noexcept {
            return m_code;
        }

        std::string_view category() const noexcept override {
            return derived::category_name();
        }

        std::string_view code_string() const noexcept override {
            return m_code_str;
        }

        std::string to_string() const noexcept override {
            return std::format("[{}:{}] {}",
                category(),
                code_string(),
                message()
            );
        }

        exception to_exception() const noexcept {
            return exception{
                std::string(category()),
                std::string(code_string()),
                std::string(message())
            };
        }
        ~typed_error() override = default;

    protected:
        enum_type m_code;
        std::string m_code_str;
        std::string m_message;
    };
}

#define ERROR_CLASS_CATEGORY(name) \
    using base = typed_error; using base::base; \
    static std::string_view category_name() noexcept { return #name; }

#define RETURN_UNEXPECTED_ERROR(category_class, type, message) \
    return std::unexpected(category_class{type, #type, message})

#define MAKE_UNEXPECTED_ERROR(category_class, type, message) \
    std::unexpected(category_class{type, #type, message})