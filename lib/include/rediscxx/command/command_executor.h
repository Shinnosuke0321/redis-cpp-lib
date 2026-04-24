//
// Created by Shinnosuke Kawai on 4/14/26.
//
#pragma once
#include "base_command.h"
#include "rediscxx/result/reply.h"
#include "rediscxx/error/exception.h"

namespace rediscxx {
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
        ~command_executor() override = default;
    private:
        static void on_handle(redisAsyncContext* ctx, void* r, void* priv) noexcept {
            const auto self = std::move(*static_cast<smart_ptr::intrusive_ptr<command_executor_base>*>(priv));
            delete static_cast<smart_ptr::intrusive_ptr<command_executor_base>*>(priv);
            auto* typed_self = static_cast<command_executor*>(self.get());

            if (auto* reply = static_cast<redisReply*>(r); !reply) {
                typed_self->m_handler(std::unexpected(error::from_ctx(ctx)));
            }
            else if (reply->type == REDIS_REPLY_ERROR) {
                using namespace error;
                using types::CommandFailed;
                typed_self->m_handler(std::unexpected(CREATE_ERROR(redis_exception, CommandFailed, reply->str)));
            }
            else {
                typed_self->m_handler(result::reply::from_raw(reply));
            }
        }

    private:
        internal::arg_buffer m_args;
        on_handler m_handler = nullptr;
    };

    template<class CmdType, class ...Args>
    requires std::derived_from<CmdType, base_command>
    smart_ptr::intrusive_ptr<command_executor_base> make_executor(Args&&... args) {
        return smart_ptr::intrusive_ptr<command_executor_base>(
            new command_executor<CmdType>(std::forward<Args>(args)...)
        );
    }
}