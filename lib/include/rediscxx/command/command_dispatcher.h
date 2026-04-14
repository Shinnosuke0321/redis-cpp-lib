//
// Created by Shinnosuke Kawai on 4/13/26.
//
#pragma once
#include "base_command.h"
#include "set_command.h"

namespace rediscxx {

    class command_dispatcher {
        using command_result_fn = std::function<void(std::expected<result::reply, error::redis_exception>)>;
        using executor_fn = std::function<void(smart_ptr::intrusive_ptr<command_executor_base>)>;
    public:
        explicit command_dispatcher(const command cmd)
        : m_cmd(cmd) {}

        void dispatch(internal::arg_buffer&& buffer, command_result_fn&& handler, executor_fn&& fn) {
            switch (m_cmd) {
                case command::auth:
                    fn(smart_ptr::make_intrusive<command_executor<auth_command>>(std::move(buffer), std::move(handler)));
                    break;
                case command::select:
                    fn(smart_ptr::make_intrusive<command_executor<select_command>>(std::move(buffer), std::move(handler)));
                    break;
                case command::set:
                    fn(smart_ptr::make_intrusive<command_executor<set_command>>(std::move(buffer), std::move(handler)));
                    break;
                case command::get:
                    fn(smart_ptr::make_intrusive<command_executor<get_command>>(std::move(buffer), std::move(handler)));
                    break;
                case command::hset:
                    fn(smart_ptr::make_intrusive<command_executor<hset_command>>(std::move(buffer), std::move(handler)));
                    break;
                case command::hget:
                    fn(smart_ptr::make_intrusive<command_executor<hget_command>>(std::move(buffer), std::move(handler)));
                    break;
                case command::hgetall:
                    fn(smart_ptr::make_intrusive<command_executor<hgetall_command>>(std::move(buffer), std::move(handler)));
                    break;
                case command::sadd:
                    fn(smart_ptr::make_intrusive<command_executor<sadd_command>>(std::move(buffer), std::move(handler)));
                    break;
                case command::srem:
                    fn(smart_ptr::make_intrusive<command_executor<srem_command>>(std::move(buffer), std::move(handler)));
                    break;
                case command::smembers:
                    fn(smart_ptr::make_intrusive<command_executor<smembers_command>>(std::move(buffer), std::move(handler)));
                    break;
                case command::exat:
                    fn(smart_ptr::make_intrusive<command_executor<exat_command>>(std::move(buffer), std::move(handler)));
                    break;
                case command::flushdb:
                    fn(smart_ptr::make_intrusive<command_executor<flush_db_command>>(std::move(buffer), std::move(handler)));
                    break;
                default:
                    break;
            }
        }
    private:
        command m_cmd;
    };
}