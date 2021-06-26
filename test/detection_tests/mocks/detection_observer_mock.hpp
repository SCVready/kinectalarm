#include <gmock/gmock.h>

#include "../../../inc/detection.hpp"

using ::testing::_;
using ::testing::SaveArg;
using ::testing::SaveArgPointee;
using ::testing::SetArgPointee;
using ::testing::DoAll;
using ::testing::Return;
using ::testing::IsNull;
using ::testing::NotNull;

class DetectionObserverMock : public DetectionObserver
{
public:

    DetectionObserverMock();
    virtual ~DetectionObserverMock();

    MOCK_METHOD(void, IntrusionStarted, ());
    MOCK_METHOD(void, IntrusionStopped, (uint32_t frame_num));
    MOCK_METHOD(void, IntrusionFrame, (KinectVideoFrame& frame, uint32_t frame_num));
};
