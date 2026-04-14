//
// Created by Shinnosuke Kawai on 4/4/26.
//
#include "rediscxx/event/pipeline_event.h"
#include "suites/pipeline_suite.h"

TEST_F(PipelineTest, InsertKeys) {
    CREATE_CLIENT(m_config);
    for (int i = 0; i < key_count; ++i) {
        client.append(rediscxx::command::set, keys[i], values[i]);
    }
    auto pl_result = client.execute_pipeline().get();
    ASSERT_TRUE(pl_result.has_value());
    auto &replies = pl_result.value();
    ASSERT_EQ(replies.size(), key_count);
    for (auto& reply : replies) {
        ASSERT_TRUE(reply.as_string().has_value());
        ASSERT_EQ(reply.as_string().value(), "OK");
    }
}

TEST_F(PipelineTest, RetriveKeys) {
    CREATE_CLIENT(m_config);
    for (auto& key : keys) {
        client.append(rediscxx::command::get, key);
    }
    const auto pl_result = client.execute_pipeline().get();
    ASSERT_TRUE(pl_result.has_value());
    auto &replies = pl_result.value();
    ASSERT_EQ(replies.size(), key_count);
    for (size_t i = 0; i < replies.size(); ++i) {
        ASSERT_TRUE(replies[i].as_string().has_value());
        ASSERT_EQ(replies[i].as_string().value(), values[i]);
    }
}

TEST_F(PipelineTest, DeletingKeys) {
    CREATE_CLIENT(m_config);
    for (auto& key : keys) {
        client.append(rediscxx::command::del, key);
    }
    const auto pl_result = client.execute_pipeline().get();
    ASSERT_TRUE(pl_result.has_value());
    auto &replies = pl_result.value();
    ASSERT_EQ(replies.size(), key_count);
    for (const auto & reply : replies) {
        ASSERT_TRUE(reply.as_integer().has_value());
        ASSERT_EQ(reply.as_integer().value(), 1);
    }
}

int main(int argc, char *argv[]) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
};