#include <gmock/gmock.h>

#include "../../../inc/message_broker_interface.hpp"

class MessageBrokerMock : public IMessageBroker
{
public:

    MessageBrokerMock() {};
    ~MessageBrokerMock() {};

    MOCK_METHOD(int, Subscribe, (const std::string& channel, const std::shared_ptr<IChannelMessageObserver> observer));
    MOCK_METHOD(int, Unsubscribe, (const std::string& channel, const std::shared_ptr<IChannelMessageObserver> observer));
    MOCK_METHOD(int, Publish, (const std::string& channel, const std::string& message));
    MOCK_METHOD(int, GetVariable, (Variable& variable));
    MOCK_METHOD(int, SetVariable, (const Variable& variable));
    MOCK_METHOD(int, SetVariableExpiration, (const Variable& variable, int livetime_seconds));
    MOCK_METHOD(int, Clear, ());
};
