//
// Created by Shinnosuke Kawai on 4/13/26.
//
#pragma once
#include "base_command.h"

namespace rediscxx {
    class auth_command: public base_command {
    public:
        COMMAND_CLASS_TYPE(auth);
        ~auth_command() override = default;
    };
    class select_command: public base_command {
    public:
        COMMAND_CLASS_TYPE(select);
        ~select_command() override = default;
    };
    class flush_db_command : public base_command {
    public:
        COMMAND_CLASS_TYPE(flushdb)
        ~flush_db_command() override = default;
    };
    class set_command: public base_command {
    public:
        COMMAND_CLASS_TYPE(set)
        ~set_command() override = default;
    };
    class get_command: public base_command {
    public:
        COMMAND_CLASS_TYPE(get);
        ~get_command() override = default;
    };
    class hget_command : public base_command {
    public:
        COMMAND_CLASS_TYPE(hget);
        ~hget_command() override = default;
    };

    class hgetall_command : public base_command {
    public:
        COMMAND_CLASS_TYPE(hgetall);
        ~hgetall_command() override = default;
    };

    class hset_command : public base_command {
    public:
        COMMAND_CLASS_TYPE(hset)
        ~hset_command() override = default;
    };

    class sadd_command: public base_command {
    public:
        COMMAND_CLASS_TYPE(sadd)
        ~sadd_command() override = default;
    };

    class smembers_command: public base_command {
    public:
        COMMAND_CLASS_TYPE(smembers)
        ~smembers_command() override = default;
    };
    class srem_command: public base_command {
    public:
        COMMAND_CLASS_TYPE(srem)
        ~srem_command() override = default;
    };
    class exat_command: public base_command {
    public:
        COMMAND_CLASS_TYPE(exat);
        ~exat_command() override = default;
    };
    class expire_command: public base_command {
    public:
        COMMAND_CLASS_TYPE(expire);
        ~expire_command() override = default;
    };
    class expireat_command: public base_command {
    public:
        COMMAND_CLASS_TYPE(expireat);
        ~expireat_command() override = default;
    };
    class incr_command: public base_command {
    public:
        COMMAND_CLASS_TYPE(incr);
        ~incr_command() override = default;
    };
}
