//
// Created by Shinnosuke Kawai on 4/13/26.
//
#pragma once
#include "base_command.h"

namespace rediscxx {
    class auth_command: public base_command {
    public:
        COMMAND_CLASS_TYPE(auth, AuthCommand);
    };
    class select_command: public base_command {
    public:
        COMMAND_CLASS_TYPE(select, SelectCommand);
    };
    class flush_db_command : public base_command {
    public:
        COMMAND_CLASS_TYPE(flushdb, FlushDbCommand)
    };
    class set_command: public base_command {
    public:
        COMMAND_CLASS_TYPE(set, SetCommand)
    };
    class get_command: public base_command {
    public:
        COMMAND_CLASS_TYPE(get, GetCommand);
    };
    class hget_command : public base_command {
    public:
        COMMAND_CLASS_TYPE(hget, HGetCommand);
    };

    class hgetall_command : public base_command {
    public:
        COMMAND_CLASS_TYPE(hgetall, HGetAllCommand);
    };

    class hset_command : public base_command {
    public:
        COMMAND_CLASS_TYPE(hset, HSetCommand)
    };

    class sadd_command: public base_command {
    public:
        COMMAND_CLASS_TYPE(sadd, SAddCommand)
    };

    class smembers_command: public base_command {
    public:
        COMMAND_CLASS_TYPE(smembers, SMembersCommand)
    };
    class srem_command: public base_command {
    public:
        COMMAND_CLASS_TYPE(srem, SRemCommand)
    };
    class exat_command: public base_command {
    public:
        COMMAND_CLASS_TYPE(exat, ExatCommand);
        ~exat_command() override = default;
    };
}
