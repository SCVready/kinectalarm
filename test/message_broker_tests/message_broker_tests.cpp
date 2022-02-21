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
#include "mocks/message_broker_observer_mock.hpp"

/*******************************************************************
 * Defines
 *******************************************************************/
#ifndef REDIS_UNIX_SOCKET
    #define REDIS_UNIX_SOCKET "/var/run/redis/redis-server.sock"
#endif

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
    MessageBroker message_broker{REDIS_UNIX_SOCKET};
    std::shared_ptr<ChannelMessageObserverMock> channel_observer_mock, channel_observer_mock_2;

    MessageBrokerTest()
    {
        channel_observer_mock = std::make_shared<ChannelMessageObserverMock>();
        channel_observer_mock_2 = std::make_shared<ChannelMessageObserverMock>();
    }

    ~MessageBrokerTest()
    {
        message_broker.Clear();
    }

protected:
};

/*******************************************************************
 * Test cases
 *******************************************************************/
TEST_F(MessageBrokerTest, Contructor)
{
    ASSERT_NO_THROW(
        MessageBroker message_broker(REDIS_UNIX_SOCKET);
    );
}

TEST_F(MessageBrokerTest, Subscribe)
{
    EXPECT_CALL(*channel_observer_mock, ChannelMessageListener(_)).Times(0);
    EXPECT_EQ(0, message_broker.Subscribe("test", channel_observer_mock));
    std::this_thread::sleep_for (std::chrono::milliseconds(5));
}

TEST_F(MessageBrokerTest, SubscribeAndPublish)
{
    std::string message("testing");
    EXPECT_CALL(*channel_observer_mock, ChannelMessageListener(message)).Times(1);
    EXPECT_EQ(0, message_broker.Subscribe("test", channel_observer_mock));
    std::this_thread::sleep_for (std::chrono::milliseconds(5));
    EXPECT_EQ(0, message_broker.Publish("test", message));
    std::this_thread::sleep_for (std::chrono::milliseconds(5));
}

TEST_F(MessageBrokerTest, TwoSubscribers)
{
    std::string message("testing");
    EXPECT_CALL(*channel_observer_mock, ChannelMessageListener(message)).Times(1);
    EXPECT_CALL(*channel_observer_mock_2, ChannelMessageListener(message)).Times(1);
    EXPECT_EQ(0, message_broker.Subscribe("test", channel_observer_mock));
    EXPECT_EQ(0, message_broker.Subscribe("test", channel_observer_mock_2));
    std::this_thread::sleep_for (std::chrono::milliseconds(5));
    EXPECT_EQ(0, message_broker.Publish("test", message));
    std::this_thread::sleep_for (std::chrono::milliseconds(5));
}

TEST_F(MessageBrokerTest, TwoChannels)
{
    std::string message("testing1");
    std::string message2("testing2");
    EXPECT_CALL(*channel_observer_mock, ChannelMessageListener(message)).Times(1);
    EXPECT_CALL(*channel_observer_mock_2, ChannelMessageListener(message2)).Times(1);
    EXPECT_EQ(0, message_broker.Subscribe("test1", channel_observer_mock));
    EXPECT_EQ(0, message_broker.Subscribe("test2", channel_observer_mock_2));
    std::this_thread::sleep_for (std::chrono::milliseconds(5));
    EXPECT_EQ(0, message_broker.Publish("test1", message));
    EXPECT_EQ(0, message_broker.Publish("test2", message2));
    std::this_thread::sleep_for (std::chrono::milliseconds(5));
}

TEST_F(MessageBrokerTest, Unsubscribe)
{
    std::string message("testing");
    EXPECT_CALL(*channel_observer_mock, ChannelMessageListener(message)).Times(1);
    EXPECT_EQ(0, message_broker.Subscribe("test", channel_observer_mock));
    std::this_thread::sleep_for (std::chrono::milliseconds(5));
    EXPECT_EQ(0, message_broker.Publish("test", message));
    std::this_thread::sleep_for (std::chrono::milliseconds(5));
    EXPECT_EQ(0, message_broker.Unsubscribe("test", channel_observer_mock));
    std::this_thread::sleep_for (std::chrono::milliseconds(5));
    EXPECT_CALL(*channel_observer_mock, ChannelMessageListener(_)).Times(0);
    EXPECT_EQ(0, message_broker.Publish("test", message));
    std::this_thread::sleep_for (std::chrono::milliseconds(5));
}

TEST_F(MessageBrokerTest, TwosubscribersOneUnsubscribe)
{
    std::string message("testing");
    EXPECT_CALL(*channel_observer_mock, ChannelMessageListener(message)).Times(1);
    EXPECT_CALL(*channel_observer_mock_2, ChannelMessageListener(message)).Times(1);
    EXPECT_EQ(0, message_broker.Subscribe("test", channel_observer_mock));
    EXPECT_EQ(0, message_broker.Subscribe("test", channel_observer_mock_2));
    std::this_thread::sleep_for (std::chrono::milliseconds(5));
    EXPECT_EQ(0, message_broker.Publish("test", message));
    std::this_thread::sleep_for (std::chrono::milliseconds(5));

    EXPECT_EQ(0, message_broker.Unsubscribe("test", channel_observer_mock));
    std::this_thread::sleep_for (std::chrono::milliseconds(5));
    EXPECT_CALL(*channel_observer_mock, ChannelMessageListener(_)).Times(0);
    EXPECT_CALL(*channel_observer_mock_2, ChannelMessageListener(message)).Times(1);
    EXPECT_EQ(0, message_broker.Publish("test", message));
    std::this_thread::sleep_for (std::chrono::milliseconds(5));
}


TEST_F(MessageBrokerTest, GetEmptyVar)
{
    Variable var{"testvar", DataType::Integer, 0};
    EXPECT_NE(0, message_broker.GetVariable(var));
}

TEST_F(MessageBrokerTest, GetWithConvertError)
{
    Variable var{"testvar", DataType::String, std::string("asd")};
    Variable var2{"testvar", DataType::Integer, 21};
    EXPECT_EQ(0, message_broker.SetVariable(var));
    EXPECT_NE(0, message_broker.GetVariable(var2));
}

TEST_F(MessageBrokerTest, SetGetVariable_Integer)
{
    Variable var{"testvar", DataType::Integer, 10};
    Variable var2{"testvar", DataType::Integer, 0};

    EXPECT_EQ(0, message_broker.SetVariable(var));
    EXPECT_EQ(0, message_broker.GetVariable(var2));

    EXPECT_EQ(std::get<int>(var.value),std::get<int>(var2.value));
}

TEST_F(MessageBrokerTest, SetGetVariable_Float)
{
    Variable var{"testvar", DataType::Float, 10.10f};
    Variable var2{"testvar", DataType::Float, 0};

    EXPECT_EQ(0, message_broker.SetVariable(var));
    EXPECT_EQ(0, message_broker.GetVariable(var2));

    EXPECT_EQ(std::get<float>(var.value),std::get<float>(var2.value));
}

TEST_F(MessageBrokerTest, SetGetVariable_String)
{
    Variable var{"testvar", DataType::String, std::string("hello")};
    Variable var2{"testvar", DataType::String, std::string()};

    EXPECT_EQ(0, message_broker.SetVariable(var));
    EXPECT_EQ(0, message_broker.GetVariable(var2));

    EXPECT_EQ(std::get<std::string>(var.value),std::get<std::string>(var2.value));
}

TEST_F(MessageBrokerTest, SetGetVariable_Bool)
{
    Variable var{"testvar", DataType::Boolean, true};
    Variable var2{"testvar", DataType::Boolean, false};

    EXPECT_EQ(0, message_broker.SetVariable(var));
    EXPECT_EQ(0, message_broker.GetVariable(var2));

    EXPECT_EQ(std::get<bool>(var.value),std::get<bool>(var2.value));
}

TEST_F(MessageBrokerTest, SetGetVariableExpiration)
{
    Variable var{"testvar", DataType::Integer, 10};
    Variable var2{"testvar", DataType::Integer, 0};

    EXPECT_EQ(0, message_broker.SetVariableExpiration(var,1));
    EXPECT_EQ(0, message_broker.GetVariable(var2));

    EXPECT_EQ(std::get<int>(var.value), std::get<int>(var2.value));

    std::this_thread::sleep_for (std::chrono::milliseconds(1100));

    EXPECT_NE(0, message_broker.GetVariable(var2));
}