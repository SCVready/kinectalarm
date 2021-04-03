#include <gmock/gmock.h>

#include "../../../inc/kinect_interface.hpp"

using ::testing::_;
using ::testing::SaveArg;
using ::testing::SaveArgPointee;
using ::testing::SetArgPointee;
using ::testing::DoAll;
using ::testing::Return;
using ::testing::IsNull;
using ::testing::NotNull;

class KinectMock : public IKinect
{
public:

    KinectMock();
    virtual ~KinectMock();

    MOCK_METHOD(int, Init, ());
    MOCK_METHOD(int, Term, ());
    MOCK_METHOD(int, Start, ());
    MOCK_METHOD(int, Stop, ());
    MOCK_METHOD(bool, IsRunning, ());
    MOCK_METHOD(void, GetDepthFrame, (KinectDepthFrame& frame));
    MOCK_METHOD(void, GetVideoFrame, (KinectVideoFrame& frame));
    MOCK_METHOD(int, ChangeTilt, (double tilt_angle));
    MOCK_METHOD(int, ChangeLedColor, (freenect_led_options color));
};
