#include <gmock/gmock.h>

#include "../../../inc/liveview.hpp"

using ::testing::_;
using ::testing::SaveArg;
using ::testing::SaveArgPointee;
using ::testing::SetArgPointee;
using ::testing::DoAll;
using ::testing::Return;
using ::testing::IsNull;
using ::testing::NotNull;

class LiveviewObserverMock : public LiveviewObserver
{
public:

    LiveviewObserverMock();
    virtual ~LiveviewObserverMock();

    MOCK_METHOD(void, NewFrame, (KinectVideoFrame& frame));
};
