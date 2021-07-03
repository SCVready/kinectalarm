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
using testing::InSequence;

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
    std::shared_ptr<Alarm> m_alarm;
    const Entry m_status_table_definition = {
        {"ID",              DataType::Integer,},
        {"TILT",            DataType::Integer,},
        {"BRIGHTNESS",      DataType::Integer,},
        {"CONTRAST",        DataType::Integer,},
        {"DET_ACTIVE",      DataType::Integer,},
        {"LVW_ACTIVE",      DataType::Integer,},
        {"CURRENT_DET_NUM", DataType::Integer,},
        {"DET_THRESHOLD",   DataType::Integer,},
        {"DET_SENSITIVITY", DataType::Integer,},
        {"DET_COOLDOWN_MS", DataType::Integer,},
        {"DET_REFRESH_REFERENCE_INTERVAL_MS", DataType::Integer,},
        {"DET_TAKE_DEPTH_FRAME_INTERVAL_MS",  DataType::Integer,},
        {"DET_TAKE_VIDEO_FRAME_INTERVAL_MS",  DataType::Integer,},
        {"LVW_VIDEO_FRAME_INTERVAL_MS",       DataType::Integer,}
    };

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

        m_alarm = std::make_shared<Alarm>(m_message_broker_mock, m_data_base_mock);
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

    void AlarmInit()
    {
        InSequence seq;

        /* InitStatePersistenceVars */
        EXPECT_CALL(*g_state_persistence_factory_mock, CreateDatatable(_, "DETECTIONS", _)).
            WillOnce(Return(g_detection_datatable_mock));
        EXPECT_CALL(*g_state_persistence_factory_mock, CreateDatatable(_, "STATUS", _)).
            WillOnce(Return(g_status_datatable_mock));
        EXPECT_CALL(*g_status_datatable_mock, GetItem(_)).
            WillOnce(Return(0));

        /* InitVarsRedis */
        EXPECT_CALL(*m_message_broker_mock, SetVariable(_)).Times(8).
            WillRepeatedly(Return(0));

        EXPECT_CALL(*g_kinect_mock, Init).
            WillOnce(Return(0));

        /* UpdateLed */
        EXPECT_CALL(*g_detection_mock, IsRunning).
            WillOnce(Return(false));
        EXPECT_CALL(*g_liveview_mock, IsRunning).
            WillOnce(Return(false));
        EXPECT_CALL(*g_kinect_mock, ChangeLedColor(_)).
            WillOnce(Return(0));

        EXPECT_CALL(*g_kinect_mock, ChangeTilt(_)).
            WillOnce(Return(0));

        EXPECT_EQ(0, m_alarm->Init());
    }

    void ExpectDetectionStartCalls()
    {

    }

    void SetDetectionActiveOnStatusTable(Entry& entry)
    {
        entry[4].value = 1;
    }

    void SetLiveviewActiveOnStatusTable(Entry& entry)
    {
        entry[5].value = 1;
    }

    void FakeIntrusion()
    {
        InSequence seq;
        AlarmDetectionObserver detection_observer(*m_alarm);

        /* UpdateLed */
        EXPECT_CALL(*g_detection_mock, IsRunning).
            WillRepeatedly(Return(false));
        EXPECT_CALL(*g_liveview_mock, IsRunning).
            WillRepeatedly(Return(false));
        EXPECT_CALL(*g_kinect_mock, ChangeLedColor(_)).
            WillOnce(Return(0));

        EXPECT_CALL(*m_message_broker_mock, Publish("new_det", _)).
            WillOnce(Return(0));
        EXPECT_CALL(*g_detection_datatable_mock, InsertItem(_)).
            WillOnce(Return(0));
        EXPECT_CALL(*m_message_broker_mock, SetVariable(_)).
            WillOnce(Return(0));
        EXPECT_CALL(*g_status_datatable_mock, SetItem(_)).
            WillOnce(Return(0));

        detection_observer.IntrusionStopped(1);
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

TEST_F(AlarmTest, InitDetectionIsActive)
{
    InSequence seq;
    Entry status_variables = m_status_table_definition;
    SetDetectionActiveOnStatusTable(status_variables);

    Alarm alarm(m_message_broker_mock, m_data_base_mock);

    /* InitStatePersistenceVars */
    EXPECT_CALL(*g_state_persistence_factory_mock, CreateDatatable(_, "DETECTIONS", _)).
        WillOnce(Return(g_detection_datatable_mock));
    EXPECT_CALL(*g_state_persistence_factory_mock, CreateDatatable(_, "STATUS", _)).
        WillOnce(Return(g_status_datatable_mock));
    EXPECT_CALL(*g_status_datatable_mock, GetItem(_)).
        WillOnce(DoAll(SetArgReferee<0>(status_variables), Return(0)));

    /* InitVarsRedis */
    EXPECT_CALL(*m_message_broker_mock, SetVariable(_)).Times(8).
        WillRepeatedly(Return(0));

    EXPECT_CALL(*g_kinect_mock, Init).
        WillOnce(Return(0));

    /* UpdateLed */
    EXPECT_CALL(*g_detection_mock, IsRunning).
        WillOnce(Return(false));
    EXPECT_CALL(*g_liveview_mock, IsRunning).
        WillOnce(Return(false));
    EXPECT_CALL(*g_kinect_mock, ChangeLedColor(_)).
        WillOnce(Return(0));

    /* StartDetection */
    EXPECT_CALL(*g_kinect_mock, IsRunning).
        WillOnce(Return(false));
    EXPECT_CALL(*g_kinect_mock, Start).
        WillOnce(Return(0));
    EXPECT_CALL(*g_detection_mock, IsRunning).
        WillOnce(Return(false));
    EXPECT_CALL(*g_detection_mock, Start).
        WillOnce(Return(0));
    EXPECT_CALL(*g_status_datatable_mock, SetItem(_)).
        WillOnce(Return(0));
    EXPECT_CALL(*m_message_broker_mock, SetVariable(_)).
        WillOnce(Return(0));

    /* UpdateLed */
    EXPECT_CALL(*g_detection_mock, IsRunning).
        WillOnce(Return(true));
    EXPECT_CALL(*g_kinect_mock, ChangeLedColor(_)).
        WillOnce(Return(0));

    EXPECT_CALL(*m_message_broker_mock, Publish(REDIS_EVENT_INFO_CHANNEL, "Detection started")).
        WillOnce(Return(0));

    EXPECT_CALL(*g_kinect_mock, ChangeTilt(_)).
        WillOnce(Return(0));

    EXPECT_EQ(0, alarm.Init());
}

TEST_F(AlarmTest, InitLiveviewIsActive)
{
    InSequence seq;
    Entry status_variables = m_status_table_definition;
    SetLiveviewActiveOnStatusTable(status_variables);

    Alarm alarm(m_message_broker_mock, m_data_base_mock);

    /* InitStatePersistenceVars */
    EXPECT_CALL(*g_state_persistence_factory_mock, CreateDatatable(_, "DETECTIONS", _)).
        WillOnce(Return(g_detection_datatable_mock));
    EXPECT_CALL(*g_state_persistence_factory_mock, CreateDatatable(_, "STATUS", _)).
        WillOnce(Return(g_status_datatable_mock));
    EXPECT_CALL(*g_status_datatable_mock, GetItem(_)).
        WillOnce(DoAll(SetArgReferee<0>(status_variables), Return(0)));

    /* InitVarsRedis */
    EXPECT_CALL(*m_message_broker_mock, SetVariable(_)).Times(8).
        WillRepeatedly(Return(0));

    EXPECT_CALL(*g_kinect_mock, Init).
        WillOnce(Return(0));

    /* UpdateLed */
    EXPECT_CALL(*g_detection_mock, IsRunning).
        WillOnce(Return(false));
    EXPECT_CALL(*g_liveview_mock, IsRunning).
        WillOnce(Return(false));
    EXPECT_CALL(*g_kinect_mock, ChangeLedColor(_)).
        WillOnce(Return(0));

    /* StartLiveview */
    EXPECT_CALL(*g_kinect_mock, IsRunning).
        WillOnce(Return(false));
    EXPECT_CALL(*g_kinect_mock, Start).
        WillOnce(Return(0));
    EXPECT_CALL(*g_liveview_mock, IsRunning).
        WillOnce(Return(false));
    EXPECT_CALL(*g_liveview_mock, Start).
        WillOnce(Return(0));
    EXPECT_CALL(*g_status_datatable_mock, SetItem(_)).
        WillOnce(Return(0));
    EXPECT_CALL(*m_message_broker_mock, SetVariable(_)).
        WillOnce(Return(0));

    /* UpdateLed */
    EXPECT_CALL(*g_detection_mock, IsRunning).
        WillOnce(Return(false));
    EXPECT_CALL(*g_liveview_mock, IsRunning).
        WillOnce(Return(true));
    EXPECT_CALL(*g_kinect_mock, ChangeLedColor(_)).
        WillOnce(Return(0));

    EXPECT_CALL(*m_message_broker_mock, Publish(REDIS_EVENT_INFO_CHANNEL, "Liveview started")).
        WillOnce(Return(0));

    EXPECT_CALL(*g_kinect_mock, ChangeTilt(_)).
        WillOnce(Return(0));

    EXPECT_EQ(0, alarm.Init());
}

TEST_F(AlarmTest, StartDetection)
{
    InSequence seq;
    AlarmInit();

    EXPECT_CALL(*g_kinect_mock, IsRunning).
        WillOnce(Return(false));
    EXPECT_CALL(*g_kinect_mock, Start).
        WillOnce(Return(0));
    EXPECT_CALL(*g_detection_mock, IsRunning).
        WillOnce(Return(false));
    EXPECT_CALL(*g_detection_mock, Start).
        WillOnce(Return(0));
    EXPECT_CALL(*g_status_datatable_mock, SetItem(_)).
        WillOnce(Return(0));
    EXPECT_CALL(*m_message_broker_mock, SetVariable(_)).
        WillOnce(Return(0));

    /* UpdateLed */
    EXPECT_CALL(*g_detection_mock, IsRunning).
        WillOnce(Return(true));
    EXPECT_CALL(*g_kinect_mock, ChangeLedColor(_)).
        WillOnce(Return(0));

    EXPECT_CALL(*m_message_broker_mock, Publish(REDIS_EVENT_INFO_CHANNEL, "Detection started")).
        WillOnce(Return(0));

    EXPECT_EQ(0, m_alarm->StartDetection());
}

TEST_F(AlarmTest, StopDetection)
{
    InSequence seq;
    AlarmInit();

    EXPECT_CALL(*g_detection_mock, IsRunning).
        WillOnce(Return(true));
    EXPECT_CALL(*g_detection_mock, Stop).
        WillOnce(Return(0));
    EXPECT_CALL(*g_status_datatable_mock, SetItem(_)).
        WillOnce(Return(0));
    EXPECT_CALL(*m_message_broker_mock, SetVariable(_)).
        WillOnce(Return(0));

    /* UpdateLed */
    EXPECT_CALL(*g_detection_mock, IsRunning).
        WillOnce(Return(false));
    EXPECT_CALL(*g_liveview_mock, IsRunning).
        WillOnce(Return(false));
    EXPECT_CALL(*g_kinect_mock, ChangeLedColor(_)).
        WillOnce(Return(0));

    EXPECT_CALL(*m_message_broker_mock, Publish(REDIS_EVENT_INFO_CHANNEL, "Detection stopped")).
        WillOnce(Return(0));

    EXPECT_CALL(*g_detection_mock, IsRunning).
        WillOnce(Return(false));
    EXPECT_CALL(*g_liveview_mock, IsRunning).
        WillOnce(Return(false));
    EXPECT_CALL(*g_kinect_mock, Stop).
        WillOnce(Return(0));

    EXPECT_EQ(0, m_alarm->StopDetection());
}

TEST_F(AlarmTest, StartLiveview)
{
    InSequence seq;
    AlarmInit();

    EXPECT_CALL(*g_kinect_mock, IsRunning).
        WillOnce(Return(false));
    EXPECT_CALL(*g_kinect_mock, Start).
        WillOnce(Return(0));
    EXPECT_CALL(*g_liveview_mock, IsRunning).
        WillOnce(Return(false));
    EXPECT_CALL(*g_liveview_mock, Start).
        WillOnce(Return(0));
    EXPECT_CALL(*g_status_datatable_mock, SetItem(_)).
        WillOnce(Return(0));
    EXPECT_CALL(*m_message_broker_mock, SetVariable(_)).
        WillOnce(Return(0));

    /* UpdateLed */
    EXPECT_CALL(*g_detection_mock, IsRunning).
        WillOnce(Return(false));
    EXPECT_CALL(*g_liveview_mock, IsRunning).
        WillOnce(Return(true));
    EXPECT_CALL(*g_kinect_mock, ChangeLedColor(_)).
        WillOnce(Return(0));

    EXPECT_CALL(*m_message_broker_mock, Publish(REDIS_EVENT_INFO_CHANNEL, "Liveview started")).
        WillOnce(Return(0));

    EXPECT_EQ(0, m_alarm->StartLiveview());
}

TEST_F(AlarmTest, StopLiveview)
{
    InSequence seq;
    AlarmInit();

    EXPECT_CALL(*g_liveview_mock, IsRunning).
        WillOnce(Return(true));
    EXPECT_CALL(*g_liveview_mock, Stop).
        WillOnce(Return(0));
    EXPECT_CALL(*g_status_datatable_mock, SetItem(_)).
        WillOnce(Return(0));
    EXPECT_CALL(*m_message_broker_mock, SetVariable(_)).
        WillOnce(Return(0));

    /* UpdateLed */
    EXPECT_CALL(*g_detection_mock, IsRunning).
        WillOnce(Return(false));
    EXPECT_CALL(*g_liveview_mock, IsRunning).
        WillOnce(Return(true));
    EXPECT_CALL(*g_kinect_mock, ChangeLedColor(_)).
        WillOnce(Return(0));

    EXPECT_CALL(*m_message_broker_mock, Publish(REDIS_EVENT_INFO_CHANNEL, "Liveview stopped")).
        WillOnce(Return(0));

    EXPECT_CALL(*g_detection_mock, IsRunning).
        WillOnce(Return(false));
    EXPECT_CALL(*g_liveview_mock, IsRunning).
        WillOnce(Return(false));
    EXPECT_CALL(*g_kinect_mock, Stop).
        WillOnce(Return(0));

    EXPECT_EQ(0, m_alarm->StopLiveview());
}


TEST_F(AlarmTest, GetNumDetections)
{
    InSequence seq;
    AlarmInit();

    EXPECT_EQ(0, m_alarm->GetNumDetections());
    FakeIntrusion();
    EXPECT_EQ(1, m_alarm->GetNumDetections());
    FakeIntrusion();
    EXPECT_EQ(2, m_alarm->GetNumDetections());
}

/*TESTs for ResetDetection */
/*TESTs for DeleteDetection */

TEST_F(AlarmTest, ChangeTilt)
{
    double tilt_value = 20;
    InSequence seq;
    AlarmInit();

    EXPECT_CALL(*g_kinect_mock, ChangeTilt(tilt_value)).
        WillOnce(Return(0));
    EXPECT_CALL(*m_message_broker_mock, SetVariable(_)).
        WillOnce(Return(0));
    EXPECT_CALL(*g_status_datatable_mock, SetItem(_)).
        WillOnce(Return(0));

    EXPECT_EQ(0, m_alarm->ChangeTilt(tilt_value));
}

TEST_F(AlarmTest, ChangeBrightness)
{
    int32_t value = 20;
    InSequence seq;
    AlarmInit();

    EXPECT_CALL(*m_message_broker_mock, SetVariable(_)).
        WillOnce(Return(0));
    EXPECT_CALL(*g_status_datatable_mock, SetItem(_)).
        WillOnce(Return(0));

    EXPECT_EQ(0, m_alarm->ChangeBrightness(value));
}

TEST_F(AlarmTest, ChangeContrast)
{
    int32_t value = 20;
    InSequence seq;
    AlarmInit();

    EXPECT_CALL(*m_message_broker_mock, SetVariable(_)).
        WillOnce(Return(0));
    EXPECT_CALL(*g_status_datatable_mock, SetItem(_)).
        WillOnce(Return(0));

    EXPECT_EQ(0, m_alarm->ChangeContrast(value));
}

TEST_F(AlarmTest, ChangeThreshold)
{
    int32_t value = 20;
    InSequence seq;
    AlarmInit();

    EXPECT_CALL(*g_detection_mock, UpdateConfig(_));
    EXPECT_CALL(*m_message_broker_mock, SetVariable(_)).
        WillOnce(Return(0));
    EXPECT_CALL(*g_status_datatable_mock, SetItem(_)).
        WillOnce(Return(0));
    EXPECT_CALL(*m_message_broker_mock, Publish(REDIS_EVENT_SUCCESS_CHANNEL, _)).
        WillOnce(Return(0));

    EXPECT_EQ(0, m_alarm->ChangeThreshold(value));
}

TEST_F(AlarmTest, ChangeSensitivity)
{
    int32_t value = 20;
    InSequence seq;
    AlarmInit();

    EXPECT_CALL(*g_detection_mock, UpdateConfig(_));
    EXPECT_CALL(*m_message_broker_mock, SetVariable(_)).
        WillOnce(Return(0));
    EXPECT_CALL(*g_status_datatable_mock, SetItem(_)).
        WillOnce(Return(0));
    EXPECT_CALL(*m_message_broker_mock, Publish(REDIS_EVENT_SUCCESS_CHANNEL, _)).
        WillOnce(Return(0));

    EXPECT_EQ(0, m_alarm->ChangeSensitivity(value));
}

TEST_F(AlarmTest, NewFrame)
{
    KinectVideoFrame frame(1080, 1080);
    Alarm alarm(m_message_broker_mock, m_data_base_mock);
    AlarmLiveviewObserver liveview_observer(alarm);

    EXPECT_CALL(*m_message_broker_mock, Publish("liveview", _)).
        WillOnce(Return(0));

    liveview_observer.NewFrame(frame);
}

TEST_F(AlarmTest, IntrusionStarted)
{
    AlarmInit();
    AlarmDetectionObserver detection_observer(*m_alarm);

    EXPECT_CALL(*m_message_broker_mock, Publish("email_send_det", _)).
        WillOnce(Return(0));
    EXPECT_CALL(*m_message_broker_mock, Publish(REDIS_EVENT_ERROR_CHANNEL, _)).
        WillOnce(Return(0));
    EXPECT_CALL(*g_kinect_mock, ChangeLedColor(LED_RED)).
        WillOnce(Return(0));

    detection_observer.IntrusionStarted();
}

TEST_F(AlarmTest, IntrusionStopped)
{
    AlarmInit();
    AlarmDetectionObserver detection_observer(*m_alarm);

    /* UpdateLed */
    EXPECT_CALL(*g_detection_mock, IsRunning).
        WillRepeatedly(Return(false));
    EXPECT_CALL(*g_liveview_mock, IsRunning).
        WillRepeatedly(Return(false));
    EXPECT_CALL(*g_kinect_mock, ChangeLedColor(_)).
        WillOnce(Return(0));

    EXPECT_CALL(*m_message_broker_mock, Publish("new_det", _)).
        WillOnce(Return(0));
    EXPECT_CALL(*m_message_broker_mock, SetVariable(_)).
        WillOnce(Return(0));
    EXPECT_CALL(*g_detection_datatable_mock, InsertItem(_)).
        WillOnce(Return(0));
    EXPECT_CALL(*g_status_datatable_mock, SetItem(_)).
        WillOnce(Return(0));

    detection_observer.IntrusionStopped(1);
}

TEST_F(AlarmTest, IntrusionFrame)
{
    std::shared_ptr<KinectVideoFrame> frame = std::make_shared<KinectVideoFrame>(1080, 1080);
    AlarmInit();
    AlarmDetectionObserver detection_observer(*m_alarm);

    detection_observer.IntrusionFrame(frame, 1);
}
