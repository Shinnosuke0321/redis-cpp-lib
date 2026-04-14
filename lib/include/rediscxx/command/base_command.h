//
// Created by Shinnosuke Kawai on 3/28/26.
//
#pragma once
#include <string>
#include <hiredis/async.h>
#include <functional>
#include "core/memory/intrusive_ptr.h"
#include "rediscxx/internal/arg_buffer.h"
#include "rediscxx/result/reply.h"
#include "rediscxx/error/exception.h"

namespace rediscxx {
    enum class command {
        set, get, hset, hget, hgetall, auth, select,
        srem, sadd, smembers, exat, expireat, flushall, flushdb, del,
        multi, exec, discard, watch
    };

    class command_executor_base : public core::ref_counted<command_executor_base> {
    public:
        virtual void execute(redisAsyncContext*) = 0;
        ~command_executor_base() override = default;
    };

    class base_command {
    public:
        virtual ~base_command() = default;
        virtual command get_cmd_type() const noexcept = 0;
        virtual std::string to_string() const noexcept = 0;
    };

#define COMMAND_CLASS_TYPE(cmd, cmd_err) \
    static std::string_view get_static_str() noexcept { return #cmd; } \
    static command get_static_type() noexcept { return command::cmd; } \
    static std::string get_err_str() noexcept { return #cmd_err; } \
    static error::types get_err_type() noexcept { return error::types::cmd_err; } \
    command get_cmd_type() const noexcept override { return get_static_type(); }; \
    std::string to_string() const noexcept override { return std::string(get_static_str()); } \

    template<typename derived_cmd_type>
    requires std::derived_from<derived_cmd_type, base_command>
    class command_executor : public command_executor_base {
        using on_handler = std::function<void(std::expected<result::reply, error::redis_exception>)>;
    public:
        explicit command_executor(internal::arg_buffer&& args, on_handler&& handler)
        : m_args(std::move(args)),
          m_handler(std::move(handler)) {
            m_args.append_at_front(derived_cmd_type::get_static_str());
        }
        void execute(redisAsyncContext *ctx) override {
            const auto argc = m_args.argc();
            auto argv = m_args.argv();
            const auto argvlen = m_args.argvlen();
            if (auto intrusive_self = this->intrusive_from_this(); redisAsyncCommandArgv(ctx, on_handle, new smart_ptr::intrusive_ptr<command_executor_base>(std::move(intrusive_self)), argc, argv.data(), argvlen.data()) != REDIS_OK) {
                m_handler(std::unexpected(error::from_ctx(ctx)));
            }
        }
    private:
        static void on_handle(redisAsyncContext* ctx, void* r, void* priv) noexcept {
            const auto self = std::move(*static_cast<smart_ptr::intrusive_ptr<command_executor_base>*>(priv));
            delete static_cast<smart_ptr::intrusive_ptr<command_executor_base>*>(priv);
            auto* typed_self = static_cast<command_executor<derived_cmd_type>*>(self.get());
            if (const auto* reply = static_cast<redisReply*>(r); !reply) {
                typed_self->m_handler(std::unexpected(error::from_ctx(ctx)));
            }
            else if (reply->type == REDIS_REPLY_ERROR) {
                typed_self->m_handler(
                    std::unexpected(error::redis_exception(derived_cmd_type::get_err_type(), derived_cmd_type::get_err_str(), reply->str))
                );
            }
            else {
                typed_self->m_handler(result::reply::from_raw(reply));
            }
        }

    private:
        internal::arg_buffer m_args;
        on_handler m_handler = nullptr;
    };
}
