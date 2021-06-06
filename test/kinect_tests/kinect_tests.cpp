/**
 * @author Alejandro Solozabal
 *
 * @file kinect_test.cpp
 *
 */

/*******************************************************************
 * Includes
 *******************************************************************/
#include <future>
#include <functional>
#include <atomic>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "../../inc/kinect.hpp"
#include "mocks/libfreenect_mock.hpp"

/*******************************************************************
 * Defines
 *******************************************************************/
#undef KINECT_GETFRAMES_TIMEOUT_MS
#define KINECT_GETFRAMES_TIMEOUT_MS 10
#define KINECT_FRAMES_UPDATE_CYCLE_MS 1

/*******************************************************************
 * Test class definition
 *******************************************************************/
using ::testing::_;
using ::testing::Return; 
using ::testing::NiceMock;
using ::testing::StrictMock;

MockLibFreenect *libfreenect_mock;
volatile bool kinect_update_frame = true;

class KinectTest : public ::testing::Test
{
public:
    Kinect kinect;
    std::thread update_frames_thread;

    KinectTest() : kinect(KINECT_GETFRAMES_TIMEOUT_MS)
    {
        libfreenect_mock = new NiceMock<MockLibFreenect>();
    }

    ~KinectTest()
    {
        kinect.Term();
        delete static_cast<NiceMock<MockLibFreenect>*>(libfreenect_mock);
    }

    void SetKinectsLastDepthFrame(KinectDepthFrame& frame)
    {
        libfreenect_mock->m_depth_cb(libfreenect_mock->m_dev,
                                     const_cast<void*>(reinterpret_cast<const void*>(frame.GetDataPointer())),
                                     frame.GetTimestamp());
    }

    void StartUpdatingKinectsLastDepthFrame(KinectDepthFrame& frame)
    {
        auto func = [this, &frame]
        {
            while(kinect_update_frame)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(KINECT_FRAMES_UPDATE_CYCLE_MS));
                SetKinectsLastDepthFrame(frame);
            }
        };
        update_frames_thread = std::thread(func);
    }

    void StopUpdatingKinectsLastDepthFrame()
    {
        kinect_update_frame = false;
        update_frames_thread.join();
    }

private:
};


/*******************************************************************
 * Test cases
 *******************************************************************/
TEST_F(KinectTest, Contructor)
{
    ASSERT_NO_THROW(
        Kinect kinect();
    );
}

TEST_F(KinectTest, InitializationSuccess)
{
    ASSERT_EQ(kinect.Init(), 0);
}

TEST_F(KinectTest, InitializationFailsOnFreenectInit)
{
    EXPECT_CALL(*libfreenect_mock, freenect_init(_,_)).
        WillOnce(Return(-1));

    ASSERT_NE(kinect.Init(), 0);
}

TEST_F(KinectTest, InitializationFailsOnFreenectNumDevices)
{
    EXPECT_CALL(*libfreenect_mock, freenect_num_devices(_)).
        WillOnce(Return(-1));

    ASSERT_NE(kinect.Init(), 0);
}

TEST_F(KinectTest, InitializationNoDevicesFound)
{
    EXPECT_CALL(*libfreenect_mock, freenect_num_devices(_)).
        WillOnce(Return(0));

    ASSERT_NE(kinect.Init(), 0);
}

TEST_F(KinectTest, InitializationFailsOnFreenectOpenDevice)
{
    EXPECT_CALL(*libfreenect_mock, freenect_open_device(_,_,_)).
        WillOnce(Return(-1));

    ASSERT_NE(kinect.Init(), 0);
}

TEST_F(KinectTest, InitializationFailsOnFreenectSetDeptMode)
{
    EXPECT_CALL(*libfreenect_mock, freenect_set_depth_mode(_,_)).
        WillOnce(Return(-1));

    ASSERT_NE(kinect.Init(), 0);
}

TEST_F(KinectTest, InitializationFailsOnFreenectSetVideoMode)
{
    EXPECT_CALL(*libfreenect_mock, freenect_set_video_mode(_,_)).
        WillOnce(Return(-1));

    ASSERT_NE(kinect.Init(), 0);
}

TEST_F(KinectTest, InitializationMultipleCallsSuccess)
{
    EXPECT_EQ(kinect.Init(), 0);
    ASSERT_EQ(kinect.Init(), 0);
}

TEST_F(KinectTest, TermSuccess)
{
    EXPECT_EQ(kinect.Init(), 0);

    EXPECT_CALL(*libfreenect_mock, freenect_close_device(_)).
        WillOnce(Return(0));
    EXPECT_CALL(*libfreenect_mock, freenect_shutdown(_)).
        WillOnce(Return(0));

    ASSERT_EQ(kinect.Term(), 0);
}

TEST_F(KinectTest, TermWithoutInitSuccess)
{
    ASSERT_EQ(kinect.Term(), 0);
}

TEST_F(KinectTest, StartAndStopSuccess)
{
    ASSERT_EQ(kinect.Init(), 0);

    EXPECT_CALL(*libfreenect_mock, freenect_process_events(_)).
        WillRepeatedly(Return(0));

    ASSERT_EQ(kinect.Start(), 0);

    std::this_thread::sleep_for(std::chrono::milliseconds(1));

    ASSERT_EQ(kinect.Stop(), 0);
}

TEST_F(KinectTest, StartFailsOnFreenectStartVideo)
{
    ASSERT_EQ(kinect.Init(), 0);

    EXPECT_CALL(*libfreenect_mock, freenect_start_video(_)).
        WillOnce(Return(-1));

    ASSERT_NE(kinect.Start(), 0);
}

TEST_F(KinectTest, StartFailsOnFreenectStartDepth)
{
    ASSERT_EQ(kinect.Init(), 0);

    EXPECT_CALL(*libfreenect_mock, freenect_start_depth(_)).
        WillOnce(Return(-1));

    ASSERT_NE(kinect.Start(), 0);
}

TEST_F(KinectTest, StopFailsOnFreenectStopDepth)
{
    ASSERT_EQ(kinect.Init(), 0);

    ASSERT_EQ(kinect.Start(), 0);

    EXPECT_CALL(*libfreenect_mock, freenect_stop_depth(_)).
        WillOnce(Return(-1));

    ASSERT_NE(kinect.Stop(), 0);
}

TEST_F(KinectTest, StopFailsOnFreenectStopVideo)
{
    ASSERT_EQ(kinect.Init(), 0);

    ASSERT_EQ(kinect.Start(), 0);

    EXPECT_CALL(*libfreenect_mock, freenect_stop_video(_)).
        WillOnce(Return(-1));

    ASSERT_NE(kinect.Stop(), 0);
}

TEST_F(KinectTest, GetDepthFrameWithDifferentTimestamp)
{
    KinectDepthFrame depth_frame(DEPTH_WIDTH, DEPTH_HEIGHT);
    KinectDepthFrame test_depth_frame(DEPTH_WIDTH, DEPTH_HEIGHT);

    depth_frame.SetTimestamp(1111);
    test_depth_frame.SetTimestamp(2222);

    ASSERT_EQ(kinect.Init(), 0);
    ASSERT_EQ(kinect.Start(), 0);

    SetKinectsLastDepthFrame(test_depth_frame);

    kinect.GetDepthFrame(depth_frame);

    EXPECT_EQ(depth_frame.GetTimestamp(), 2222);

    ASSERT_EQ(kinect.Stop(), 0);
}

TEST_F(KinectTest, GetDepthFrameWithSameTimestampTimeout)
{
    KinectDepthFrame depth_frame(DEPTH_WIDTH, DEPTH_HEIGHT);
    KinectDepthFrame test_depth_frame(DEPTH_WIDTH, DEPTH_HEIGHT);

    depth_frame.SetTimestamp(1111);
    test_depth_frame.SetTimestamp(1111);

    ASSERT_EQ(kinect.Init(), 0);
    ASSERT_EQ(kinect.Start(), 0);

    SetKinectsLastDepthFrame(test_depth_frame);

    kinect.GetDepthFrame(depth_frame);

    ASSERT_EQ(kinect.Stop(), 0);
}

TEST_F(KinectTest, GetDepthFrameWithSameTimestampWaitsNextFrame)
{
    KinectDepthFrame depth_frame(DEPTH_WIDTH, DEPTH_HEIGHT);
    KinectDepthFrame initial_depth_frame(DEPTH_WIDTH, DEPTH_HEIGHT);
    KinectDepthFrame updated_depth_frame(DEPTH_WIDTH, DEPTH_HEIGHT);

    depth_frame.SetTimestamp(1111);
    initial_depth_frame.SetTimestamp(1111);
    updated_depth_frame.SetTimestamp(2222);

    ASSERT_EQ(kinect.Init(), 0);
    ASSERT_EQ(kinect.Start(), 0);

    EXPECT_CALL(*libfreenect_mock, freenect_process_events(_)).
        WillRepeatedly(Return(0));

    SetKinectsLastDepthFrame(initial_depth_frame);

    StartUpdatingKinectsLastDepthFrame(updated_depth_frame);

    kinect.GetDepthFrame(depth_frame);

    StopUpdatingKinectsLastDepthFrame();

    EXPECT_EQ(depth_frame.GetTimestamp(), 2222);

    EXPECT_EQ(kinect.Stop(), 0);
}

TEST_F(KinectTest, ChangeTiltSuccess)
{
    ASSERT_EQ(kinect.Init(), 0);

    EXPECT_CALL(*libfreenect_mock, freenect_set_tilt_degs(_,_)).
        WillOnce(Return(0));

    ASSERT_EQ(kinect.ChangeTilt(10), 0);
}

TEST_F(KinectTest, ChangeTiltFailsOnFreenectSetTiltDegs)
{
    ASSERT_EQ(kinect.Init(), 0);

    EXPECT_CALL(*libfreenect_mock, freenect_set_tilt_degs(_,_)).
        WillOnce(Return(-1));

    ASSERT_NE(kinect.ChangeTilt(10), 0);
}

TEST_F(KinectTest, ChangeLedColorSuccess)
{
    ASSERT_EQ(kinect.Init(), 0);

    EXPECT_CALL(*libfreenect_mock, freenect_set_led(_,LED_GREEN)).
        WillOnce(Return(0));

    ASSERT_EQ(kinect.ChangeLedColor(LED_GREEN), 0);
}

TEST_F(KinectTest, ChangeLedColorFailsOnFreenectSetTiltDegs)
{
    ASSERT_EQ(kinect.Init(), 0);

    EXPECT_CALL(*libfreenect_mock, freenect_set_led(_,LED_GREEN)).
        WillOnce(Return(-1));

    ASSERT_NE(kinect.ChangeLedColor(LED_GREEN), 0);
}
