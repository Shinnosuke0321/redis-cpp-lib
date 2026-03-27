//
// Created by Shinnosuke Kawai on 2/24/26.
//
#pragma once
#include <string>
#include <string_view>
#include <format>

namespace database {
    struct redis_error {
        enum class type {
            ConnectFailed, AuthFailed, SelectDbFailed, CommandFailed,
            ReplyNull, ReplyError, InvalidArgument, AppendFailed, ContextFailed
        };
        static redis_error FailedToConnect() noexcept {return redis_error{type::ConnectFailed, "Failed to connect to redis"};}
        static redis_error AuthFailed() noexcept {return redis_error{type::AuthFailed, "Failed to authenticate to redis"};}
        static redis_error ContextErr(const std::string& msg = {}) noexcept {
            return redis_error{type::ContextFailed, msg.empty() ? "Failed to create redis context" : msg};
        }
        static redis_error CommandFailed() noexcept {return redis_error{type::CommandFailed, "Failed to execute redis command"};}
        static redis_error ReplyError(const char* err) noexcept {return redis_error{type::ReplyError, err};}
        static redis_error ReplyNull() noexcept {return redis_error{type::ReplyNull, "Redis reply is null"};}
        static redis_error AppendFailed() noexcept {return redis_error{type::AppendFailed, "Failed to append redis command"};}

        std::string to_str() const noexcept {
            std::string_view code_str;
            switch (code) {
                case type::ConnectFailed:
                    code_str = "ConnectFailed";
                    break;
                case type::AuthFailed:
                    code_str = "AuthFailed";
                    break;
                case type::SelectDbFailed:
                    code_str = "SelectDbFailed";
                    break;
                case type::CommandFailed:
                    code_str = "CommandFailed";
                    break;
                case type::ReplyNull:
                    code_str = "ReplyNull";
                    break;
                case type::ReplyError:
                    code_str = "ReplyError";
                    break;
                case type::InvalidArgument:
                    code_str = "InvalidArgument";
                    break;
                case type::AppendFailed:
                    code_str = "AppendFailed";
                    break;
                case type::ContextFailed:
                    code_str = "ContextFailed";
                    break;
            }
            return std::format("Redis: {}: {}", code_str, message);
        }
    private:
        type code{};
        std::string message;
        redis_error() = default;
        redis_error(const type code, const char * str): code(code), message(str){}
        redis_error(const type code, std::string str): code(code), message(std::move(str)){}
        redis_error(const type code, std::string&& str): code(code), message(std::move(str)){}
    };
}