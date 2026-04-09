//
// Created by Shinnosuke Kawai on 11/3/25.
//

#include "rediscxx/client.h"

namespace rediscxx {
    client::~client() = default;

    std::expected<void, Core::Database::ConnectionError> client::connect() const noexcept {
        using Core::Database::ConnectionError;

        if (auto res = m_event_executer->init(); !res) {
            return std::unexpected(ConnectionError::ConnectionFailed(res.error().to_str().c_str()));
        }

        std::promise<std::expected<void, ConnectionError>> promise;
        auto future = promise.get_future();
        m_transport->connect_async(m_config.host, m_config.port, m_config.password, m_config.dbIndex, [&promise](const std::expected<void, connection_error> &result) {
            if (!result) {
                promise.set_value(std::unexpected());
            }
        });
        return future.get();
    }

}
