#include <gmock/gmock.h>

#include <libfreenect/libfreenect.h>
#include <libfreenect/libfreenect_sync.h>

using ::testing::_;
using ::testing::SaveArg;
using ::testing::SaveArgPointee;
using ::testing::SetArgPointee;
using ::testing::DoAll;
using ::testing::Return;
using ::testing::IsNull;
using ::testing::NotNull;

class MockLibFreenect
{
public:
    freenect_context* m_ctx;
    freenect_device* m_dev;
    freenect_raw_tilt_state m_tilt_state;
    freenect_frame_mode m_frame_mode;
    freenect_device_attributes m_attribute_list;
    freenect_depth_cb m_depth_cb;
    freenect_video_cb m_video_cb;

    MockLibFreenect();
    virtual ~MockLibFreenect();

    MOCK_METHOD(void, freenect_set_log_level, (freenect_context *ctx, freenect_loglevel level));
    MOCK_METHOD(void, freenect_select_subdevices, (freenect_context *ctx, freenect_device_flags subdevs));
    MOCK_METHOD(int, freenect_num_devices, (freenect_context *ctx));
    MOCK_METHOD(int, freenect_open_device, (freenect_context *ctx, freenect_device **dev, int index));
    MOCK_METHOD(freenect_frame_mode, freenect_find_depth_mode, (freenect_resolution res, freenect_depth_format fmt));
    MOCK_METHOD(int, freenect_set_depth_mode, (freenect_device* dev, const freenect_frame_mode mode));
    MOCK_METHOD(freenect_frame_mode, freenect_find_video_mode, (freenect_resolution res, freenect_video_format fmt));
    MOCK_METHOD(int, freenect_set_video_mode, (freenect_device* dev, freenect_frame_mode mode));
    MOCK_METHOD(void, freenect_set_depth_callback, (freenect_device *dev, freenect_depth_cb cb));
    MOCK_METHOD(void, freenect_set_video_callback, (freenect_device *dev, freenect_video_cb cb));
    MOCK_METHOD(int, freenect_shutdown, (freenect_context *ctx));
    MOCK_METHOD(int, freenect_process_events, (freenect_context *ctx));
    MOCK_METHOD(int, freenect_init, (freenect_context **ctx, freenect_usb_context *usb_ctx));
    MOCK_METHOD(int, freenect_close_device, (freenect_device *dev));
    MOCK_METHOD(int, freenect_start_video, (freenect_device *dev));
    MOCK_METHOD(int, freenect_stop_depth, (freenect_device *dev));
    MOCK_METHOD(int, freenect_set_led, (freenect_device *dev, freenect_led_options option));
    MOCK_METHOD(int, freenect_start_depth, (freenect_device *dev));
    MOCK_METHOD(int, freenect_stop_video, (freenect_device *dev));
    MOCK_METHOD(int, freenect_update_tilt_state, (freenect_device *dev));
    MOCK_METHOD(int, freenect_set_tilt_degs, (freenect_device *dev, double angle));
    MOCK_METHOD(freenect_raw_tilt_state*, freenect_get_tilt_state, (freenect_device *dev));
    MOCK_METHOD(int, freenect_list_device_attributes, (freenect_context *ctx, struct freenect_device_attributes** attribute_list));
    MOCK_METHOD(void, freenect_free_device_attributes, (struct freenect_device_attributes* attribute_list));
};
