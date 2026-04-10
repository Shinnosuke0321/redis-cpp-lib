//
// Created by Shinnosuke Kawai on 11/3/25.
//

#include "rediscxx/client.h"

namespace rediscxx {
    client::~client() = default;

    std::expected<void, core::connection_error> client::connect() const noexcept {

        if (auto res = m_event_executer->init(); !res) {
            return std::unexpected(CONNECTION_ERROR(ConnectionFailed, res.error().what()));
        }

        std::promise<std::expected<void, ConnectionError>> promise;
        auto future = promise.get_future();
        m_transport->connect_async(m_config.host, m_config.port, m_config.password, m_config.dbIndex, [&promise](const std::expected<void, redis_exception> &result) {
            if (!result) {
                promise.set_value(std::unexpected());
            }
        });
        return future.get();
    }

}
