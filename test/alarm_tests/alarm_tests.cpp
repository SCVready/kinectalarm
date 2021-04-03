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

#include "../common/mocks/kinect_mock.hpp"
#include "../common/mocks/message_broker_mock.hpp"
#include "mocks/alarm_module_mock.hpp"
#include "../../inc/alarm.hpp"

/*******************************************************************
 * Test class definition
 *******************************************************************/
using ::testing::_;
using ::testing::Return; 
using ::testing::NiceMock;
using ::testing::StrictMock;
using ::testing::SetArgReferee;
using ::testing::Ref;

std::shared_ptr<KinectMock> kinect_mock;
std::shared_ptr<AlarmModuleMock> detection_mock;
std::shared_ptr<AlarmModuleMock> liveview_mock;

class AlarmTest : public ::testing::Test
{
public:
    AlarmTest()
    {
        kinect_mock    = std::make_shared<StrictMock<KinectMock>>();
        detection_mock = std::make_shared<StrictMock<AlarmModuleMock>>();
        liveview_mock  = std::make_shared<StrictMock<AlarmModuleMock>>();
        m_message_broker_mock = std::make_shared<NiceMock<MessageBrokerMock>>();
    }

    ~AlarmTest()
    {
    }
protected:
    std::shared_ptr<IMessageBroker> m_message_broker_mock;
};

/*******************************************************************
 * Test cases
 *******************************************************************/
TEST_F(AlarmTest, Contructor)
{
    ASSERT_NO_THROW(
        Alarm alarm(m_message_broker_mock);
    );
}

TEST_F(AlarmTest, Init)
{
    Alarm alarm(m_message_broker_mock);

    EXPECT_CALL(*kinect_mock, Init).
        WillOnce(Return(0));
    EXPECT_CALL(*kinect_mock, IsRunning).
        WillRepeatedly(Return(false));
    EXPECT_CALL(*detection_mock, IsRunning).
        WillRepeatedly(Return(false));
    EXPECT_CALL(*liveview_mock, IsRunning).
        WillRepeatedly(Return(false));
    EXPECT_CALL(*kinect_mock, ChangeLedColor(_)).
        WillRepeatedly(Return(0));
    EXPECT_CALL(*kinect_mock, ChangeTilt(_)).
        WillRepeatedly(Return(0));

    alarm.Init();
}
