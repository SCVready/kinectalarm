/**
 * @author Alejandro Solozabal
 *
 * @file cKinect.cpp
 *
 */

/*******************************************************************
 * Includes
 *******************************************************************/
#include <chrono>

#include "log.hpp"
#include "kinect.hpp"

/*******************************************************************
 * Static variables
 *******************************************************************/
std::unique_ptr<KinectFrame> Kinect::m_depth_frame;
std::unique_ptr<KinectFrame> Kinect::m_video_frame;

std::mutex Kinect::m_depth_mutex, Kinect::m_video_mutex;
std::condition_variable Kinect::m_depth_cv, Kinect::m_video_cv;

uint32_t Kinect::m_timeout_ms;

/*******************************************************************
 * Class definition
 *******************************************************************/
Kinect::Kinect(uint32_t timeout_ms) : CyclicTask("Kinect", 0)
{
    /* Members initialization */
    m_is_kinect_initialized = false;
    m_kinect_ctx            = NULL;
    m_kinect_dev            = NULL;
    m_timeout_ms            = timeout_ms;

    m_depth_frame = std::make_unique<KinectFrame>(DEPTH_WIDTH, DEPTH_HEIGHT);
    m_video_frame = std::make_unique<KinectFrame>(VIDEO_WIDTH, VIDEO_HEIGHT);

}

Kinect::~Kinect()
{
}

int Kinect::Init()
{
    int retval = -1;

    /* Check if it's already initialize */
    if(m_is_kinect_initialized)
    {
        LOG(LOG_INFO,"Kinect is already initialized\n");
        retval = 0;
    }
    else
    {
        /* Library freenect init */
        if (0 != freenect_init(&m_kinect_ctx, nullptr))
        {
            LOG(LOG_ERR,"freenect_init() failed\n");
        }
        else
        {
            /* Set log level */
            freenect_set_log_level(m_kinect_ctx, FREENECT_LOG_FATAL);

            /* Select subdevices */
            freenect_select_subdevices(m_kinect_ctx, (freenect_device_flags) (FREENECT_DEVICE_MOTOR | FREENECT_DEVICE_CAMERA));

            /* Find out how many devices are connected */
            int num_devices = 0;
            if (0 > (num_devices = freenect_num_devices(m_kinect_ctx)))
            {
                LOG(LOG_ERR,"freenect_num_devices() failed\n");
            }
            else if(num_devices == 0)
            {
                LOG(LOG_ERR,"No Kinect device found\n");
            }
            else
            {
                freenect_device_attributes* attribute_list;

                LOG(LOG_INFO,"Kinect device found\n");

                if (1 > freenect_list_device_attributes(m_kinect_ctx, &attribute_list))
                {
                    LOG(LOG_WARNING,"freenect_list_device_attributes() failed\n");
                }
                else
                {
                    LOG(LOG_INFO,"Kinect device with camera_serial: %s\n", attribute_list->camera_serial);
                    freenect_free_device_attributes(attribute_list);
                }

                /* Open the first device */
                if (0 != freenect_open_device(m_kinect_ctx, &m_kinect_dev, 0))
                {
                    LOG(LOG_ERR,"freenect_open_device() failed\n");
                }
                /* Configure depth and video mode */
                else if (0 != freenect_set_depth_mode(m_kinect_dev, freenect_find_depth_mode(FREENECT_RESOLUTION_MEDIUM, FREENECT_DEPTH_11BIT)))
                {
                    LOG(LOG_ERR,"freenect_set_depth_mode() failed\n");
                }
                else if (0 != freenect_set_video_mode(m_kinect_dev, freenect_find_video_mode(FREENECT_RESOLUTION_MEDIUM, FREENECT_VIDEO_IR_10BIT)))
                {
                    LOG(LOG_ERR,"freenect_set_video_mode() failed\n");
                }
                else
                {
                    /* Set frame callbacks */
                    freenect_set_depth_callback(m_kinect_dev, DepthCallback);
                    freenect_set_video_callback(m_kinect_dev, VideoCallback);

                    /* Set kinect init flag to true */
                    m_is_kinect_initialized = true;
                    retval = 0;

                    LOG(LOG_INFO,"Kinect initialization successful\n");
                }
            }
        }
    }
    return retval;
}

int Kinect::Term()
{
    int retval = 0;

    if(!m_is_kinect_initialized)
    {
        LOG(LOG_INFO,"Kinect is already terminated\n");
    }
    else
    {
        LOG(LOG_INFO,"Shutting down kinect\n");

        /* Stop everything and shutdown */
        if(m_kinect_dev)
        {
            if (0 != freenect_close_device(m_kinect_dev))
            {
                LOG(LOG_ERR,"freenect_close_device() failed\n");
                retval = -1;
            }
        }

        if(m_kinect_ctx)
        {
            if (0 != freenect_shutdown(m_kinect_ctx))
            {
                LOG(LOG_ERR,"freenect_shutdown() failed\n");
                retval = -1;
            }
        }

        m_is_kinect_initialized = false;
    }

    return retval;
}

int Kinect::Start()
{
    int retval = -1;

    /* Initialize frame time-stamps */
    m_depth_frame->SetTimestamp(0);
    m_video_frame->SetTimestamp(0);

    if(0 != freenect_start_video(m_kinect_dev))
    {
        LOG(LOG_ERR,"freenect_start_video() failed\n");
    }
    else if(0 != freenect_start_depth(m_kinect_dev))
    {
        LOG(LOG_ERR,"freenect_start_video() failed\n");
    }
    else if(0 != CyclicTask::Start())
    {
        LOG(LOG_ERR,"CyclicTask::Start() failed\n");
    }
    else
    {
        LOG(LOG_INFO,"Kinect started successfully\n");
        retval = 0;
    }
    

    return retval;
}

int Kinect::Stop()
{
    int retval = 0;

    /* Call parent Stop to stop the execution thread*/
    if( 0 != CyclicTask::Stop())
    {
        LOG(LOG_ERR,"CyclicTask::Stop() failed\n");
        retval = -1;
    }
    if(0 != freenect_stop_depth(m_kinect_dev))
    {
        LOG(LOG_ERR,"freenect_stop_depth() failed\n");
        retval = -1;
    }
    if(0 != freenect_stop_video(m_kinect_dev))
    {
        LOG(LOG_ERR,"freenect_stop_depth() failed\n");
        retval = -1;
    }

    if(retval == 0)
    {
        LOG(LOG_INFO,"Kinect stopped successfully\n");
    }

    return retval;
}

void Kinect::ExecutionCycle()
{
    freenect_process_events(m_kinect_ctx);
}

void Kinect::GetDepthFrame(KinectFrame& frame)
{
    std::unique_lock<std::mutex> ulock(m_depth_mutex);

    /* Compare the given timestamp with the current, if it's the same must wait to the next frame */
    if(frame.GetTimestamp() == m_depth_frame->GetTimestamp())
    {
        if(m_depth_cv.wait_for(ulock, std::chrono::milliseconds(m_timeout_ms)) == std::cv_status::timeout)
        {
            LOG(LOG_WARNING,"GetDepthFrame() failed to acquire a frame in %u ms\n", m_timeout_ms);
        }
    }

    frame = *m_depth_frame;
}

void Kinect::GetVideoFrame(KinectFrame& frame)
{
    std::unique_lock<std::mutex> ulock(m_video_mutex);

    /*  Compare the given timestamp with the current, if it's the same must wait to the next frame */
    if(frame.GetTimestamp() == m_video_frame->GetTimestamp())
    {
        if(m_video_cv.wait_for(ulock, std::chrono::milliseconds(m_timeout_ms)) == std::cv_status::timeout)
        {
            LOG(LOG_WARNING,"GetVideoFrame() failed to acquire a frame in %u ms\n", m_timeout_ms);
        }
    }

    frame = *m_video_frame;
}

void Kinect::DepthCallback(freenect_device* dev, void* data, uint32_t timestamp)
{
    std::unique_lock<std::mutex> ulock(m_depth_mutex);
    m_depth_frame->Fill(static_cast<uint16_t*>(data), timestamp);
    m_depth_cv.notify_all();
}

void Kinect::VideoCallback(freenect_device* dev, void* data, uint32_t timestamp)
{
    std::unique_lock<std::mutex> ulock(m_video_mutex);
    m_video_frame->Fill(static_cast<uint16_t*>(data), timestamp);
    m_video_cv.notify_all();
}

int Kinect::ChangeTilt(double tilt_angle)
{
    int retval = 0; 

    if(0 != freenect_set_tilt_degs(m_kinect_dev, tilt_angle))
    {
        LOG(LOG_WARNING,"freenect_set_tilt_degs() failed\n");
        retval = -1;
    }
    else
    {
        LOG(LOG_DEBUG,"Kinect::ChangeTilt() success\n");
    }

    return retval;
}

int Kinect::ChangeLedColor(freenect_led_options color)
{
    int retval = 0; 
    if(0 != freenect_set_led(m_kinect_dev,color))
    {
        LOG(LOG_WARNING,"freenect_set_led() failed\n");
        retval = -1;
    }
    else
    {
        LOG(LOG_DEBUG,"Kinect::ChangeLedColor() success\n");
    }
    return retval;
}
