//
// Created by Shinnosuke Kawai on 2/24/26.
//
#pragma once
#include <string>
#include <string_view>
#include <format>
#include <core/error/base_error.h>

namespace rediscxx {
    enum class error_code {
        connection_error, auth_failed, select_db_failed, command_failed,
        reply_null, reply_error, invalid_argument, append_failed, context_failed,
        transaction_error, event_loop_error, pipeline_error, command_error
    };

#define INITIALIZE_EXCEPTION(type) \
    explicit type(const char* msg): message(msg) {} \
    explicit type(std::string&& msg): message(std::move(msg)) {} \
    explicit type(const std::string& msg): message(msg) {} \
    ~type() override = default; \
    private: std::string message;

#define EXCEPTION_CLASS_TYPE(type) \
    std::string_view get_name() const {return #type;} \
    error_code get_err_type() const {return error_code::type;} \
    std::string to_str() const noexcept override { \
        std::string name = std::string(get_name());\
        std::transform(name.begin(), name.end(), name.begin(), ::toupper);\
        return std::format("Redis: {}: {}", std::move(name), message); \
    }

    class connection_error: public Core::BaseError {
    public:
        INITIALIZE_EXCEPTION(connection_error)
    public:
        EXCEPTION_CLASS_TYPE(connection_error)
    };

    class command_error: public Core::BaseError {
    public:
        INITIALIZE_EXCEPTION(command_error)
    public:
        EXCEPTION_CLASS_TYPE(command_error)
    };

    class auth_failed: Core::BaseError {
    public:
        INITIALIZE_EXCEPTION(auth_failed)
    public:
        EXCEPTION_CLASS_TYPE(auth_failed)
    };

    class select_db_failed: Core::BaseError {
    public:
        INITIALIZE_EXCEPTION(select_db_failed)
    public:
        EXCEPTION_CLASS_TYPE(select_db_failed)
    };

    class command_failed: Core::BaseError {
    public:
        INITIALIZE_EXCEPTION(command_failed)
    public:
        EXCEPTION_CLASS_TYPE(command_failed)
    };

    class reply_null: Core::BaseError {
    public:
        INITIALIZE_EXCEPTION(reply_null)
    public:
        EXCEPTION_CLASS_TYPE(reply_null)
    };

    class reply_error: Core::BaseError {
    public:
        INITIALIZE_EXCEPTION(reply_error)
    public:
        EXCEPTION_CLASS_TYPE(reply_error)
    };

    class invalid_argument: Core::BaseError {
    public:
        INITIALIZE_EXCEPTION(invalid_argument)
    public:
        EXCEPTION_CLASS_TYPE(invalid_argument)
    };

    class append_failed: Core::BaseError {
    public:
        INITIALIZE_EXCEPTION(append_failed)
    public:
        EXCEPTION_CLASS_TYPE(append_failed)
    };

    class context_failed: Core::BaseError {
    public:
        INITIALIZE_EXCEPTION(context_failed)
    public:
        EXCEPTION_CLASS_TYPE(context_failed)
    };

    class transaction_error: Core::BaseError {
    public:
        INITIALIZE_EXCEPTION(transaction_error)
        EXCEPTION_CLASS_TYPE(transaction_error)
    };

    class pipeline_error: Core::BaseError {
    public:
        INITIALIZE_EXCEPTION(pipeline_error)
    public:
        EXCEPTION_CLASS_TYPE(pipeline_error)
    };

    class event_loop_error: Core::BaseError {
    public:
        INITIALIZE_EXCEPTION(event_loop_error)
    public:
        EXCEPTION_CLASS_TYPE(event_loop_error)
    };
}