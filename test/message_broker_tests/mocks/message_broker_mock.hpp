#include <gmock/gmock.h>

#include "../../../inc/message_broker.hpp"

using ::testing::_;
using ::testing::SaveArg;
using ::testing::SaveArgPointee;
using ::testing::SetArgPointee;
using ::testing::DoAll;
using ::testing::Return;
using ::testing::IsNull;
using ::testing::NotNull;

class ChannelMessageObserverMock : public ChannelMessageObserver
{
public:

    ChannelMessageObserverMock();
    virtual ~ChannelMessageObserverMock();

    MOCK_METHOD(void, ChannelMessageListener, (const std::string& message));
};
