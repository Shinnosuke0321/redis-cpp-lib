//
// Created by Shinnosuke Kawai on 4/13/26.
//
#pragma once
#include <functional>
#include <expected>
#include "base_command.h"
#include "commands.h"
#include "command_executor.h"

namespace rediscxx {
    class command_dispatcher {
        using command_result_fn = std::function<void(std::expected<result::reply, error::redis_exception>)>;
        using executor_fn = std::function<void(smart_ptr::intrusive_ptr<command_executor_base>)>;
    public:
        explicit command_dispatcher(const command cmd)
        : m_cmd(cmd) {}

        void dispatch(internal::arg_buffer&& buffer, command_result_fn&& handler, executor_fn&& fn) const {
            switch (m_cmd) {
                case command::auth:
                    fn(make_executor<auth_command>(std::move(buffer), std::move(handler)));
                    break;
                case command::select:
                    fn(make_executor<select_command>(std::move(buffer), std::move(handler)));
                    break;
                case command::set:
                    fn(make_executor<set_command>(std::move(buffer), std::move(handler)));
                    break;
                case command::get:
                    fn(make_executor<get_command>(std::move(buffer), std::move(handler)));
                    break;
                case command::hset:
                    fn(make_executor<hset_command>(std::move(buffer), std::move(handler)));
                    break;
                case command::hget:
                    fn(make_executor<hget_command>(std::move(buffer), std::move(handler)));
                    break;
                case command::hgetall:
                    fn(make_executor<hgetall_command>(std::move(buffer), std::move(handler)));
                    break;
                case command::sadd:
                    fn(make_executor<sadd_command>(std::move(buffer), std::move(handler)));
                    break;
                case command::srem:
                    fn(make_executor<srem_command>(std::move(buffer), std::move(handler)));
                    break;
                case command::smembers:
                    fn(make_executor<smembers_command>(std::move(buffer), std::move(handler)));
                    break;
                case command::exat:
                    fn(make_executor<exat_command>(std::move(buffer), std::move(handler)));
                    break;
                case command::expire:
                    fn(make_executor<expire_command>(std::move(buffer), std::move(handler)));
                    break;
                case command::expireat:
                    fn(make_executor<expireat_command>(std::move(buffer), std::move(handler)));
                    break;
                case command::flushdb:
                    fn(make_executor<flush_db_command>(std::move(buffer), std::move(handler)));
                    break;
                case command::incr:
                    fn(make_executor<incr_command>(std::move(buffer), std::move(handler)));
                    break;
                default:
                    break;
            }
        }
    private:
        command m_cmd;
    };
}