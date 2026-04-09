//
// Created by Shinnosuke Kawai on 3/28/26.
//
#pragma once
#include <string>
#include <utility>
#include <hiredis/async.h>
#include <functional>
#include "rediscxx/result/reply.h"

namespace rediscxx {
    enum class command {
        set, get, hset, hgetall, sadd, smembers, auth, select,
        srem, exat, expireat, hget, flush_all, flush_db, del,
        multi, exec, discard, watch
    };

#define COMMAND_CLASS_TYPE(cmd) \
    std::string_view get_name() const noexcept override { return #cmd; } \
    command get_cmd_type() const noexcept override { return command::cmd; }; \
    std::string to_string() const noexcept override { return std::string(get_name());}

    class  command_base {
    public:
        virtual void execute(redisAsyncContext*) = 0;
        virtual ~command_base() = default;
        virtual std::string_view get_name() const noexcept = 0;
        virtual command get_cmd_type() const noexcept = 0;
        virtual std::string to_string() const noexcept = 0;

    };

    class auth_command: public command_base, public core::ref_counted<auth_command> {
    public:
        explicit auth_command(std::string password, std::function<void(std::expected<void, connection_error>)>&& handler)
        : m_password(std::move(password)),
          m_handler(std::move(handler)) {}
    public:
        COMMAND_CLASS_TYPE(auth)

        void execute(redisAsyncContext *ctx) override {
            if (auto self = this->intrusive_from_this();
                redisAsyncCommand(ctx, on_callback, new smart_ptr::intrusive_ptr(std::move(self)), "AUTH %s", m_password.c_str()) != REDIS_OK) {
                m_handler(std::unexpected(connection_error("failed to send auth")));
            }
        }

    private:
        static void on_callback(redisAsyncContext *ctx, void *r, void *priv) {
            const auto self = *static_cast<smart_ptr::intrusive_ptr<auth_command>*>(priv);
            delete static_cast<smart_ptr::intrusive_ptr<auth_command>*>(priv);
            if (const auto *reply = static_cast<redisReply*>(r); !reply) {
                self->m_handler(std::unexpected(connection_error("reply is null")));
            }
            else if (reply->type == REDIS_REPLY_ERROR) {
                self->m_handler(std::unexpected(connection_error(reply->str)));
            }
            else {
                self->m_handler({});
            }
        }
    private:
        std::string m_password;
        std::function<void(std::expected<void, connection_error>)> m_handler;
    };

    class select_command: public command_base, public core::ref_counted<select_command> {
    public:
        explicit select_command(const int index, std::function<void(std::expected<void, connection_error>)>&& handler)
        : m_index(index),
          m_handler(std::move(handler)){}
    public:
        COMMAND_CLASS_TYPE(select)

        void execute(redisAsyncContext *ctx) override {
            if (auto self = this->intrusive_from_this();
                redisAsyncCommand(ctx, on_callback, new smart_ptr::intrusive_ptr(std::move(self)), "SELECT %d", m_index) != REDIS_OK) {
                m_handler(std::unexpected(connection_error("failed to send select")));
            }
        }
        ~select_command() override = default;
    private:
        static void on_callback(redisAsyncContext *ctx, void *r, void *priv) {
            const auto* self = static_cast<select_command*>(priv);
            if (const auto *reply = static_cast<redisReply*>(r); !reply) {
                self->m_handler(std::unexpected(connection_error("reply is null")));
            }
            else if (reply->type == REDIS_REPLY_ERROR) {
                self->m_handler(std::unexpected(connection_error(reply->str)));
            }
            else {
                self->m_handler({});
            }
        }

    private:
        int m_index;
        std::function<void(std::expected<void, connection_error>)> m_handler;
    };
}
