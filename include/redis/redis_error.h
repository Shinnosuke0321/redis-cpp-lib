//
// Created by Shinnosuke Kawai on 2/24/26.
//
#pragma once
#include <string>
#include <string_view>
#include <format>

namespace Database {
    struct RedisError {
        enum class Code {
            ConnectFailed, AuthFailed, SelectDbFailed, CommandFailed,
            ReplyNull, ReplyError, InvalidArgument, AppendFailed, ContextFailed
        };
        static RedisError FailedToConnect() noexcept {return RedisError{Code::ConnectFailed, "Failed to connect to redis"};}
        static RedisError AuthFailed() noexcept {return RedisError{Code::AuthFailed, "Failed to authenticate to redis"};}
        static RedisError ContextErr(const std::string& msg = {}) noexcept {
            return RedisError{Code::ContextFailed, msg.empty() ? "Failed to create redis context" : msg};
        }
        static RedisError CommandFailed() noexcept {return RedisError{Code::CommandFailed, "Failed to execute redis command"};}
        static RedisError ReplyError(const char* err) noexcept {return RedisError{Code::ReplyError, err};}
        static RedisError ReplyNull() noexcept {return RedisError{Code::ReplyNull, "Redis reply is null"};}
        static RedisError AppendFailed() noexcept {return RedisError{Code::AppendFailed, "Failed to append redis command"};}

        std::string to_str() const noexcept {
            std::string_view code_str;
            switch (code) {
                case Code::ConnectFailed:
                    code_str = "ConnectFailed";
                    break;
                case Code::AuthFailed:
                    code_str = "AuthFailed";
                    break;
                case Code::SelectDbFailed:
                    code_str = "SelectDbFailed";
                    break;
                case Code::CommandFailed:
                    code_str = "CommandFailed";
                    break;
                case Code::ReplyNull:
                    code_str = "ReplyNull";
                    break;
                case Code::ReplyError:
                    code_str = "ReplyError";
                    break;
                case Code::InvalidArgument:
                    code_str = "InvalidArgument";
                    break;
                case Code::AppendFailed:
                    code_str = "AppendFailed";
                    break;
                case Code::ContextFailed:
                    code_str = "ContextFailed";
                    break;
            }
            return std::format("Redis: {}: {}", code_str, message);
        }
    private:
        Code code{};
        std::string message;
        RedisError() = default;
        RedisError(const Code code, const char * str): code(code), message(str){}
        RedisError(const Code code, std::string str): code(code), message(std::move(str)){}
        RedisError(const Code code, std::string&& str): code(code), message(std::move(str)){}
    };
}