#include <libfreenect/libfreenect.h>
#include "../mocks/libfreenect_mock.hpp"

extern MockLibFreenect *libfreenect_mock;

void freenect_set_log_level(freenect_context *ctx, freenect_loglevel level)
{
    return libfreenect_mock->freenect_set_log_level(ctx, level);
}

void freenect_select_subdevices(freenect_context *ctx, freenect_device_flags subdevs)
{
    return libfreenect_mock->freenect_select_subdevices(ctx, subdevs);
}

int freenect_num_devices(freenect_context *ctx)
{
    return libfreenect_mock->freenect_num_devices(ctx);
}

int freenect_open_device(freenect_context *ctx, freenect_device **dev, int index)
{
    return libfreenect_mock->freenect_open_device(ctx, dev, index);
}

freenect_frame_mode freenect_find_depth_mode(freenect_resolution res, freenect_depth_format fmt)
{
    return libfreenect_mock->freenect_find_depth_mode(res, fmt);
}

int freenect_set_depth_mode(freenect_device* dev, const freenect_frame_mode mode)
{
    return libfreenect_mock->freenect_set_depth_mode(dev, mode);
}

freenect_frame_mode freenect_find_video_mode(freenect_resolution res, freenect_video_format fmt)
{
    return libfreenect_mock->freenect_find_video_mode(res, fmt);
}

int freenect_set_video_mode(freenect_device* dev, freenect_frame_mode mode)
{
    return libfreenect_mock->freenect_set_video_mode(dev, mode);
}

void freenect_set_depth_callback(freenect_device *dev, freenect_depth_cb cb)
{
    return libfreenect_mock->freenect_set_depth_callback(dev, cb);
}

void freenect_set_video_callback(freenect_device *dev, freenect_video_cb cb)
{
    return libfreenect_mock->freenect_set_video_callback(dev, cb);
}

int freenect_shutdown(freenect_context *ctx)
{
    return libfreenect_mock->freenect_shutdown(ctx);
}

int freenect_process_events(freenect_context *ctx)
{
    return libfreenect_mock->freenect_process_events(ctx);
}

int freenect_init(freenect_context **ctx, freenect_usb_context *usb_ctx)
{
    return libfreenect_mock->freenect_init(ctx,usb_ctx);
}

int freenect_close_device(freenect_device *dev)
{
    return libfreenect_mock->freenect_close_device(dev);
}

int freenect_start_video(freenect_device *dev)
{
    return libfreenect_mock->freenect_start_video(dev);
}

int freenect_stop_depth(freenect_device *dev)
{
    return libfreenect_mock->freenect_stop_depth(dev);
}

int freenect_set_led(freenect_device *dev, freenect_led_options option)
{
    return libfreenect_mock->freenect_set_led(dev, option);
}

int freenect_start_depth(freenect_device *dev)
{
    return libfreenect_mock->freenect_start_depth(dev);
}

int freenect_stop_video(freenect_device *dev)
{
    return libfreenect_mock->freenect_stop_video(dev);
}

int freenect_update_tilt_state(freenect_device *dev)
{
    return libfreenect_mock->freenect_update_tilt_state(dev);
}

int freenect_set_tilt_degs(freenect_device *dev, double angle)
{
    return libfreenect_mock->freenect_set_tilt_degs(dev, angle);
}

freenect_raw_tilt_state* freenect_get_tilt_state(freenect_device *dev)
{
    return libfreenect_mock->freenect_get_tilt_state(dev);
}

int freenect_list_device_attributes(freenect_context *ctx, struct freenect_device_attributes** attribute_list)
{
    return libfreenect_mock->freenect_list_device_attributes(ctx, attribute_list);
}

void freenect_free_device_attributes(struct freenect_device_attributes* attribute_list)
{
    return libfreenect_mock->freenect_free_device_attributes(attribute_list);
}