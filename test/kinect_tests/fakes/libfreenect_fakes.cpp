#include <libfreenect/libfreenect.h>
#include "../mocks/libfreenect_mock.hpp"

extern MockLibFreenect *libfreenect_mock;

void freenect_set_log_level(freenect_context *ctx, freenect_loglevel level)
{
    ;
}

void freenect_select_subdevices(freenect_context *ctx, freenect_device_flags subdevs)
{
    ;
}

int freenect_num_devices(freenect_context *ctx)
{
    return libfreenect_mock->freenect_num_devices(ctx);
}

int freenect_open_device(freenect_context *ctx, freenect_device **dev, int index)
{
    return 0;
}

freenect_frame_mode freenect_find_depth_mode(freenect_resolution res, freenect_depth_format fmt)
{
    freenect_frame_mode frame_mode;
    return frame_mode;
}

int freenect_set_depth_mode(freenect_device* dev, const freenect_frame_mode mode)
{
    return 0;
}

freenect_frame_mode freenect_find_video_mode(freenect_resolution res, freenect_video_format fmt)
{
    freenect_frame_mode frame_mode;
    return frame_mode;
}

int freenect_set_video_mode(freenect_device* dev, freenect_frame_mode mode)
{
    return 0;
}

void freenect_set_depth_callback(freenect_device *dev, freenect_depth_cb cb)
{
    ;
}

void freenect_set_video_callback(freenect_device *dev, freenect_video_cb cb)
{
    ;
}

int freenect_shutdown(freenect_context *ctx)
{
    return 0;
}

int freenect_process_events(freenect_context *ctx)
{
    return 0;
}

int freenect_init(freenect_context **ctx, freenect_usb_context *usb_ctx)
{
    return libfreenect_mock->freenect_init(ctx,usb_ctx);
}

int freenect_close_device(freenect_device *dev)
{
    return 0;
}

int freenect_start_video(freenect_device *dev)
{
    return 0;
}

int freenect_stop_depth(freenect_device *dev)
{
    return 0;
}

int freenect_set_led(freenect_device *dev, freenect_led_options option)
{
    return 0;
}

int freenect_start_depth(freenect_device *dev)
{
    return 0;
}

int freenect_stop_video(freenect_device *dev)
{
    return 0;
}

int freenect_update_tilt_state(freenect_device *dev)
{
    return 0;
}

int freenect_set_tilt_degs(freenect_device *dev, double angle)
{
    return 0;
}

freenect_raw_tilt_state* freenect_get_tilt_state(freenect_device *dev)
{
    return 0;
}