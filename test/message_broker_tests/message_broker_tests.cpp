/**
 * @author Alejandro Solozabal
 *
 * @file liveview_tests.cpp
 *
 */

/*******************************************************************
 * Includes
 *******************************************************************/
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <memory>

#include "../../inc/message_broker.hpp"
#include "mocks/message_broker_mock.hpp"

/*******************************************************************
 * Test class definition
 *******************************************************************/
using ::testing::_;
using ::testing::Return; 
using ::testing::NiceMock;
using ::testing::StrictMock;
using ::testing::SetArgReferee;
using ::testing::Ref;

class MessageBrokerTest : public ::testing::Test
{
public:
    MessageBroker message_broker{"/var/run/redis/redis-server.sock"};
    std::shared_ptr<ChannelMessageObserverMock> channel_observer_mock;

    MessageBrokerTest()
    {
        channel_observer_mock = std::make_shared<ChannelMessageObserverMock>();
    }

    ~MessageBrokerTest()
    {
    }

protected:
};

/*******************************************************************
 * Test cases
 *******************************************************************/
TEST_F(MessageBrokerTest, Contructor)
{
    ASSERT_NO_THROW(
        MessageBroker message_broker("/var/run/redis/redis-server.sock");
    );
}

TEST_F(MessageBrokerTest, Subscribe)
{
    EXPECT_CALL(*channel_observer_mock, ChannelMessageListener(_)).Times(0);
    EXPECT_EQ(0, message_broker.Subscribe("test", channel_observer_mock));
}

TEST_F(MessageBrokerTest, SubscribeAndPublish)
{
    std::string message("testing");
    EXPECT_CALL(*channel_observer_mock, ChannelMessageListener(message)).Times(1);
    EXPECT_EQ(0, message_broker.Subscribe("test", channel_observer_mock));
    EXPECT_EQ(0, message_broker.Publish("test", message));
    std::this_thread::sleep_for (std::chrono::milliseconds(5)); /*TODO: why this is needed to avoid SIGABRT*/
}
