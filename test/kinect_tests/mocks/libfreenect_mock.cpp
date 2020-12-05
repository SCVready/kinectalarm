#include "libfreenect_mock.hpp"

MockLibFreenect::MockLibFreenect()
{
    m_ctx = reinterpret_cast<freenect_context*>(0x01);
    m_dev = reinterpret_cast<freenect_device*>(0x02);
    m_attribute_list.camera_serial = "kinectserial";
    m_depth_cb = nullptr;
    m_video_cb = nullptr;

    ON_CALL(*this, freenect_set_log_level(m_ctx,_)).
        WillByDefault(Return());
    ON_CALL(*this, freenect_select_subdevices(m_ctx,_)).
        WillByDefault(Return());
    ON_CALL(*this, freenect_num_devices(m_ctx)).
        WillByDefault(Return(1));
    ON_CALL(*this, freenect_open_device(m_ctx,NotNull(),_)).
        WillByDefault(DoAll(SetArgPointee<1>(m_dev),Return(0)));
    ON_CALL(*this, freenect_find_depth_mode(_,_)).
        WillByDefault(Return(m_frame_mode));
    ON_CALL(*this, freenect_set_depth_mode(_,_)).
        WillByDefault(Return(0));
    ON_CALL(*this, freenect_find_video_mode(_,_)).
        WillByDefault(Return(m_frame_mode));
    ON_CALL(*this, freenect_set_video_mode(_,_)).
        WillByDefault(Return(0));
    ON_CALL(*this, freenect_set_depth_callback(m_dev,NotNull())).
        WillByDefault(SaveArgPointee<1>(&m_depth_cb));
    ON_CALL(*this, freenect_set_video_callback(m_dev,NotNull())).
        WillByDefault(SaveArgPointee<1>(&m_video_cb));
    ON_CALL(*this, freenect_shutdown(m_ctx)).
        WillByDefault(Return(0));
    ON_CALL(*this, freenect_process_events(m_ctx)).
        WillByDefault(Return(0));
    ON_CALL(*this, freenect_init(NotNull(),IsNull())).
        WillByDefault(DoAll(SetArgPointee<0>(m_ctx),Return(0)));
    ON_CALL(*this, freenect_close_device(_)).
        WillByDefault(Return(0));
    ON_CALL(*this, freenect_start_video(_)).
        WillByDefault(Return(0));
    ON_CALL(*this, freenect_stop_depth(_)).
        WillByDefault(Return(0));
    ON_CALL(*this, freenect_set_led(_,_)).
        WillByDefault(Return(0));
    ON_CALL(*this, freenect_start_depth(_)).
        WillByDefault(Return(0));
    ON_CALL(*this, freenect_stop_video(_)).
        WillByDefault(Return(0));
    ON_CALL(*this, freenect_update_tilt_state(_)).
        WillByDefault(Return(0));
    ON_CALL(*this, freenect_set_tilt_degs(_,_)).
        WillByDefault(Return(0));
    ON_CALL(*this, freenect_get_tilt_state(_)).
        WillByDefault(Return(&m_tilt_state));
    ON_CALL(*this, freenect_list_device_attributes(m_ctx, NotNull())).
        WillByDefault(DoAll(SetArgPointee<1>(&m_attribute_list),Return(1)));
    ON_CALL(*this, freenect_free_device_attributes(&m_attribute_list)).
        WillByDefault(Return());
}

MockLibFreenect::~MockLibFreenect()
{
}