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
#include "../common/mocks/state_persistence_factory_mock.hpp"
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
using ::testing::DoAll;
using ::testing::Invoke;

std::shared_ptr<IDataTable> g_data_table_mock;
std::shared_ptr<KinectMock> g_kinect_mock;
std::shared_ptr<AlarmModuleMock> g_detection_mock;
std::shared_ptr<AlarmModuleMock> g_liveview_mock;
std::shared_ptr<StatePersistenceFactoryMock> g_state_persistence_factory_mock;
std::shared_ptr<DataTableMock> g_detection_datatable_mock;
std::shared_ptr<DataTableMock> g_status_datatable_mock;

class AlarmTest : public ::testing::Test
{
protected:
    std::shared_ptr<MessageBrokerMock> m_message_broker_mock;
    std::shared_ptr<DatabaseMock> m_data_base_mock;

public:
    AlarmTest()
    {
        g_kinect_mock                    = std::make_shared<StrictMock<KinectMock>>();
        g_detection_mock                 = std::make_shared<StrictMock<AlarmModuleMock>>();
        g_liveview_mock                  = std::make_shared<StrictMock<AlarmModuleMock>>();
        g_data_table_mock                = std::make_shared<StrictMock<DataTableMock>>();
        g_state_persistence_factory_mock = std::make_shared<StrictMock<StatePersistenceFactoryMock>>();
        g_detection_datatable_mock       = std::make_shared<StrictMock<DataTableMock>>();
        g_status_datatable_mock          = std::make_shared<StrictMock<DataTableMock>>();
        m_data_base_mock                 = std::make_shared<StrictMock<DatabaseMock>>();
        m_message_broker_mock            = std::make_shared<StrictMock<MessageBrokerMock>>();
    }

    ~AlarmTest()
    {
        g_kinect_mock.reset();
        g_detection_mock.reset();
        g_liveview_mock.reset();
        g_data_table_mock.reset();
        g_state_persistence_factory_mock.reset();
        g_detection_datatable_mock.reset();
        g_status_datatable_mock.reset();
    }
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

TEST_F(AlarmTest, InitStatusVarsRead)
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

    EXPECT_CALL(*g_state_persistence_factory_mock, CreateDatatable(_, "DETECTIONS", _)).
        WillOnce(Return(g_detection_datatable_mock));
    EXPECT_CALL(*g_state_persistence_factory_mock, CreateDatatable(_, "STATUS", _)).
        WillOnce(Return(g_status_datatable_mock));
    EXPECT_CALL(*g_status_datatable_mock, GetItem(_)).
        WillOnce(Return(0));
    EXPECT_CALL(*m_message_broker_mock, SetVariable(_)).
        WillRepeatedly(Return(0));

    EXPECT_EQ(0, alarm.Init());
}

TEST_F(AlarmTest, InitFailedCreatingDetectionPersistenceTable)
{
    Alarm alarm(m_message_broker_mock, m_data_base_mock);

    EXPECT_CALL(*g_state_persistence_factory_mock, CreateDatatable(_, "DETECTIONS", _)).
        WillOnce(Return(nullptr));

    EXPECT_NE(0, alarm.Init());
}

TEST_F(AlarmTest, InitFailedStatusPersistenceTable)
{
    Alarm alarm(m_message_broker_mock, m_data_base_mock);

    EXPECT_CALL(*g_state_persistence_factory_mock, CreateDatatable(_, "DETECTIONS", _)).
        WillOnce(Return(g_detection_datatable_mock));

    EXPECT_CALL(*g_state_persistence_factory_mock, CreateDatatable(_, "STATUS", _)).
        WillOnce(Return(nullptr));

    EXPECT_NE(0, alarm.Init());
}

TEST_F(AlarmTest, InitFailedReadingStatusTable)
{
    Alarm alarm(m_message_broker_mock, m_data_base_mock);

    EXPECT_CALL(*g_state_persistence_factory_mock, CreateDatatable(_, "DETECTIONS", _)).
        WillOnce(Return(g_detection_datatable_mock));
    EXPECT_CALL(*g_state_persistence_factory_mock, CreateDatatable(_, "STATUS", _)).
        WillOnce(Return(g_status_datatable_mock));
    EXPECT_CALL(*g_status_datatable_mock, GetItem(_)).
        WillOnce(Return(-1));
    EXPECT_CALL(*g_status_datatable_mock, InsertItem(_)).
        WillOnce(Return(0));
    EXPECT_CALL(*m_message_broker_mock, SetVariable(_)).
        WillRepeatedly(Return(0));

    EXPECT_CALL(*g_kinect_mock, Init).
        WillOnce(Return(0));
    EXPECT_CALL(*g_detection_mock, IsRunning).
        WillOnce(Return(false));
    EXPECT_CALL(*g_liveview_mock, IsRunning).
        WillOnce(Return(false));
    EXPECT_CALL(*g_kinect_mock, ChangeLedColor(_)).
        WillOnce(Return(0));
    EXPECT_CALL(*g_kinect_mock, ChangeTilt(_)).
        WillOnce(Return(0));

    EXPECT_EQ(0, alarm.Init());
}

TEST_F(AlarmTest, InitFailedKinectInit)
{
    Alarm alarm(m_message_broker_mock, m_data_base_mock);

    EXPECT_CALL(*g_state_persistence_factory_mock, CreateDatatable(_, "DETECTIONS", _)).
        WillOnce(Return(g_detection_datatable_mock));
    EXPECT_CALL(*g_state_persistence_factory_mock, CreateDatatable(_, "STATUS", _)).
        WillOnce(Return(g_status_datatable_mock));
    EXPECT_CALL(*g_status_datatable_mock, GetItem(_)).
        WillOnce(Return(-1));
    EXPECT_CALL(*g_status_datatable_mock, InsertItem(_)).
        WillOnce(Return(0));
    EXPECT_CALL(*m_message_broker_mock, SetVariable(_)).
        WillRepeatedly(Return(0));

    EXPECT_CALL(*g_kinect_mock, Init).
        WillOnce(Return(-1));

    EXPECT_NE(0, alarm.Init());
}