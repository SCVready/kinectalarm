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
#include "../common/mocks/state_persistence_mock.hpp"
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

std::shared_ptr<IDataTable> g_data_table_mock;
std::shared_ptr<KinectMock> g_kinect_mock;
std::shared_ptr<AlarmModuleMock> g_detection_mock;
std::shared_ptr<AlarmModuleMock> g_liveview_mock;

class AlarmTest : public ::testing::Test
{
public:
    AlarmTest()
    {
        g_kinect_mock      = std::make_shared<StrictMock<KinectMock>>();
        g_detection_mock   = std::make_shared<StrictMock<AlarmModuleMock>>();
        g_liveview_mock    = std::make_shared<StrictMock<AlarmModuleMock>>();
        m_data_base_mock = std::make_shared<StrictMock<DatabaseMock>>();
        m_message_broker_mock = std::make_shared<StrictMock<MessageBrokerMock>>();
        g_data_table_mock = std::make_shared<StrictMock<DataTableMock>>();
    }

    ~AlarmTest()
    {
    }

protected:
    std::shared_ptr<IMessageBroker> m_message_broker_mock;
    std::shared_ptr<IDatabase> m_data_base_mock;
};

/*******************************************************************
 * Test cases
 *******************************************************************/
TEST_F(AlarmTest, Contructor)
{
    ASSERT_NO_THROW(
        Alarm alarm(m_message_broker_mock, m_data_base_mock);
    );
}

TEST_F(AlarmTest, Init)
{
    Alarm alarm(m_message_broker_mock, m_data_base_mock);

    EXPECT_CALL(*g_kinect_mock, Init).
        WillOnce(Return(0));
    EXPECT_CALL(*g_kinect_mock, IsRunning).
        WillRepeatedly(Return(false));
    EXPECT_CALL(*g_detection_mock, IsRunning).
        WillRepeatedly(Return(false));
    EXPECT_CALL(*g_liveview_mock, IsRunning).
        WillRepeatedly(Return(false));
    EXPECT_CALL(*g_kinect_mock, ChangeLedColor(_)).
        WillRepeatedly(Return(0));
    EXPECT_CALL(*g_kinect_mock, ChangeTilt(_)).
        WillRepeatedly(Return(0));

    alarm.Init();
}
