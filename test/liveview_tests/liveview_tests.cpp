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

#include "../mocks/kinect_mock.hpp"
#include "mocks/liveview_observer_mock.hpp"
#include "../../inc/liveview.hpp"

/*******************************************************************
 * Test class definition
 *******************************************************************/
using ::testing::_;
using ::testing::Return; 
using ::testing::NiceMock;
using ::testing::StrictMock;
using ::testing::SetArgReferee;
using ::testing::Ref;

class LiveviewTest : public ::testing::Test
{
public:
    LiveviewTest()
    {
        liveview_config.video_frame_interval_ms = 100;
        kinect_mock = std::make_shared<StrictMock<KinectMock>>();
        liveview_observer_mock = std::make_shared<StrictMock<LiveviewObserverMock>>();
    }

    ~LiveviewTest()
    {
    }

protected:
    LiveviewConfig liveview_config;
    std::shared_ptr<KinectMock> kinect_mock;
    std::shared_ptr<LiveviewObserverMock> liveview_observer_mock;
};

/*******************************************************************
 * Test cases
 *******************************************************************/
TEST_F(LiveviewTest, Contructor)
{
    ASSERT_NO_THROW(
        Liveview liveview(kinect_mock, liveview_observer_mock, liveview_config);
    );
}

TEST_F(LiveviewTest, GetAndPushFrames)
{
    Liveview liveview(kinect_mock, liveview_observer_mock, liveview_config);
    KinectVideoFrame kinect_video_frame(1920,1080);

    EXPECT_CALL(*kinect_mock, GetVideoFrame(_)).
        WillRepeatedly(SetArgReferee<0>(kinect_video_frame));
    EXPECT_CALL(*liveview_observer_mock, NewFrame(_));
        /* TODO: check the content of the argument passed */

    ASSERT_EQ(liveview.Start(), 0);

    std::this_thread::sleep_for(std::chrono::milliseconds(1));

    ASSERT_EQ(liveview.Stop(), 0);
}