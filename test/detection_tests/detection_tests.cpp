/**
 * @author Alejandro Solozabal
 *
 * @file Detection_tests.cpp
 *
 */

/*******************************************************************
 * Includes
 *******************************************************************/
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "../common/mocks/kinect_mock.hpp"
#include "mocks/detection_observer_mock.hpp"
#include "../../inc/detection.hpp"

/*******************************************************************
 * Test class definition
 *******************************************************************/
using ::testing::_;
using ::testing::Return; 
using ::testing::NiceMock;
using ::testing::StrictMock;
using ::testing::SetArgReferee;
using ::testing::Ref;
using ::testing::AtLeast;

class DetectionTest : public ::testing::Test
{
public:
    DetectionTest()
    {
        detection_config.threshold = 2000;
        detection_config.sensitivity = 10;
        detection_config.cooldown_ms = 100;
        detection_config.refresh_reference_interval_ms = 40;
        detection_config.take_depth_frame_interval_ms = 10;
        detection_config.take_video_frame_interval_ms = 20;

        kinect_mock = std::make_shared<StrictMock<KinectMock>>();
        detection_observer_mock = std::make_shared<StrictMock<DetectionObserverMock>>();
    }

    ~DetectionTest()
    {
    }

    void FillFrameWithValue(KinectFrame& frame, uint16_t value, uint32_t timestamp)
    {
        std::vector<uint16_t> frame_data;
        frame_data.resize(1920*1080);
        frame_data.assign(frame_data.size(), value);
        frame.Fill(frame_data.data(), timestamp);
    }

protected:
    DetectionConfig detection_config;
    std::shared_ptr<KinectMock> kinect_mock;
    std::shared_ptr<DetectionObserverMock> detection_observer_mock;
    uint32_t loop_period_ms = 100;
};

/*******************************************************************
 * Test cases
 *******************************************************************/
TEST_F(DetectionTest, Contructor)
{
    ASSERT_NO_THROW(
        Detection detection(kinect_mock, detection_observer_mock, detection_config);
    );
}

TEST_F(DetectionTest, StartsTakingDepthFrames)
{
    Detection detection(kinect_mock, detection_observer_mock, detection_config);
    KinectDepthFrame kinect_frame_ref(1920,1080);

    EXPECT_CALL(*kinect_mock, GetDepthFrame(_)).
        WillRepeatedly(SetArgReferee<0>(kinect_frame_ref));

    ASSERT_EQ(detection.Start(), 0);

    std::this_thread::sleep_for(std::chrono::milliseconds(1));

    ASSERT_EQ(detection.Stop(), 0);
}

TEST_F(DetectionTest, DetectionOccursSuccess)
{
    Detection detection(kinect_mock, detection_observer_mock, detection_config);
    KinectDepthFrame kinect_depth_frame_ref(1920,1080);
    KinectDepthFrame kinect_depth_frame_1(1920,1080);
    KinectVideoFrame kinect_video_frame_1(1920,1080);

    FillFrameWithValue(kinect_depth_frame_ref, 100, 1);
    FillFrameWithValue(kinect_depth_frame_1, 200, 2);

    EXPECT_CALL(*kinect_mock, GetDepthFrame(_)).
        WillOnce(SetArgReferee<0>(kinect_depth_frame_ref)).
        WillRepeatedly(SetArgReferee<0>(kinect_depth_frame_1));

    EXPECT_CALL(*detection_observer_mock, IntrusionStarted()).Times(1);

    EXPECT_CALL(*kinect_mock, GetVideoFrame(_)).
        WillRepeatedly(SetArgReferee<0>(kinect_video_frame_1));

    EXPECT_CALL(*detection_observer_mock, IntrusionFrame(_, _)).Times(AtLeast(1));

    EXPECT_CALL(*detection_observer_mock, IntrusionStopped(_)).Times(1);

    ASSERT_EQ(detection.Start(), 0);

    std::this_thread::sleep_for(std::chrono::milliseconds(150));

    ASSERT_EQ(detection.Stop(), 0);
}