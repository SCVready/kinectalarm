/**
 * @author Alejandro Solozabal
 *
 * @file cKinect.cpp
 *
 */

/*******************************************************************
 * Includes
 *******************************************************************/
#include "kinect.hpp"

/*******************************************************************
 * Static variables
 *******************************************************************/
uint16_t* Kinect::temp_depth_frame_raw;
uint16_t* Kinect::temp_video_frame_raw;

uint32_t Kinect::temp_depth_frame_timestamp;
uint32_t Kinect::temp_video_frame_timestamp;

pthread_mutex_t Kinect::depth_lock;
pthread_mutex_t Kinect::video_lock;

pthread_cond_t  Kinect::depth_ready;
pthread_cond_t  Kinect::video_ready;

/*******************************************************************
 * Class definition
 *******************************************************************/
Kinect::Kinect()
{
    /* Members initialization */
    is_kinect_initialize = false;
    kinect_ctx           = NULL;
    kinect_dev           = NULL;
    running              = false;
    process_event_thread = 0;
}

Kinect::~Kinect()
{
}

int Kinect::Init()
{
    int ret = 0;

    /* Check if it's already initialize */
    if(is_kinect_initialize)
        return 0;

    /* Mutex init */
    pthread_mutex_init(&Kinect::depth_lock, NULL);
    pthread_mutex_init(&Kinect::video_lock, NULL);

    /* new frame Mutex init */
    pthread_cond_init(&Kinect::depth_ready, NULL);
    pthread_cond_init(&Kinect::video_ready, NULL);

    /* Library freenect init */
    ret = freenect_init(&kinect_ctx, NULL);
    if (ret < 0)
        return -1;

    /* Set log level */
    freenect_set_log_level(kinect_ctx, FREENECT_LOG_FATAL);

    /* Select subdevices */
    freenect_select_subdevices(kinect_ctx, (freenect_device_flags) (FREENECT_DEVICE_MOTOR | FREENECT_DEVICE_CAMERA));

    /* Find out how many devices are connected */
    int num_devices = ret = freenect_num_devices(kinect_ctx);
    if (ret < 0)
    {
        LOG(LOG_ERR,"Error searching for Kinect devices\n");
        freenect_shutdown(kinect_ctx);
        return -1;
    }

    else if (num_devices == 0)
    {
        LOG(LOG_ERR,"No Kinect device found\n");
        freenect_shutdown(kinect_ctx);
        return -1;
    }

    /* Open the first device */
    ret = freenect_open_device(kinect_ctx, &kinect_dev, 0);
    if (ret < 0)
    {
        freenect_shutdown(kinect_ctx);
        return -1;
    }

    /* Configure depth and video mode */
    ret = freenect_set_depth_mode(kinect_dev, freenect_find_depth_mode(FREENECT_RESOLUTION_MEDIUM, FREENECT_DEPTH_11BIT));
    if (ret < 0)
    {
        freenect_shutdown(kinect_ctx);
        return -1;
    }

    ret = freenect_set_video_mode(kinect_dev, freenect_find_video_mode(FREENECT_RESOLUTION_MEDIUM, FREENECT_VIDEO_IR_10BIT));
    if (ret < 0)
    {
        freenect_shutdown(kinect_ctx);
        return -1;
    }

    /* Set frame callbacks */
    freenect_set_depth_callback(kinect_dev, DepthCallback);
    freenect_set_video_callback(kinect_dev, VideoCallback);

    /* Malloc for frames */
    temp_depth_frame_raw = (uint16_t*) malloc (DEPTH_WIDTH * DEPTH_HEIGHT * sizeof(uint16_t));
    temp_video_frame_raw = (uint16_t*) malloc (VIDEO_WIDTH * VIDEO_HEIGHT * sizeof(uint16_t));

    /* Initialize frame time-stamps */
    Kinect::temp_depth_frame_timestamp = 0;
    Kinect::temp_video_frame_timestamp = 0;

    /* Set kinect init flag to true */
    is_kinect_initialize = true;

    return 0;
}

int Kinect::Term()
{
    LOG(LOG_INFO,"Shutting down kinect\n");

    /* Stop everything and shutdown */
    if(kinect_dev)
    {
        freenect_close_device(kinect_dev);
        freenect_shutdown(kinect_ctx);
    }

    /* Free memory pointers */
    free(temp_depth_frame_raw);
    free(temp_video_frame_raw);

    /* Initialize flag to false */
    is_kinect_initialize = false;

    return 0;
}

int Kinect::Start()
{
    /* Initialize frame time-stamps */
    Kinect::temp_depth_frame_timestamp = 0;
    Kinect::temp_video_frame_timestamp = 0;

    running = true;
    freenect_start_video(kinect_dev);
    freenect_start_depth(kinect_dev);
    pthread_create(&process_event_thread, 0, KinectProcessEventsHelper, this);

    return 0;
}

int Kinect::Stop()
{
    running = false;
    pthread_join(process_event_thread,NULL);
    if(kinect_dev)
    {
        freenect_stop_depth(kinect_dev);
        freenect_stop_video(kinect_dev);
    }

    return 0;
}

int Kinect::GetDepthFrame(uint16_t *depth_frame, uint32_t *timestamp)
{
    pthread_mutex_lock(&Kinect::depth_lock);

    /* Compare the given timestamp with the current, if it's the same must wait to the next frame */
    if(*timestamp == Kinect::temp_depth_frame_timestamp)
        pthread_cond_wait(&Kinect::depth_ready, &Kinect::depth_lock);

    memcpy (depth_frame, Kinect::temp_depth_frame_raw, (DEPTH_WIDTH*DEPTH_HEIGHT)*sizeof(uint16_t));
    *timestamp = Kinect::temp_depth_frame_timestamp;

    pthread_mutex_unlock(&Kinect::depth_lock);
    return 0;
}

void Kinect::GetDepthFrame_ex(std::shared_ptr<KinectFrame> frame)
{
    pthread_mutex_lock(&Kinect::depth_lock);

    /* Compare the given timestamp with the current, if it's the same must wait to the next frame */
    if(frame->m_timestamp == Kinect::temp_depth_frame_timestamp)
        pthread_cond_wait(&Kinect::depth_ready, &Kinect::depth_lock);

    frame->Fill(temp_depth_frame_raw);
    frame->m_timestamp = Kinect::temp_depth_frame_timestamp;

    pthread_mutex_unlock(&Kinect::depth_lock);
}

int Kinect::GetVideoFrame(uint16_t *video_frame, uint32_t *timestamp)
{

    pthread_mutex_lock(&Kinect::video_lock);

    /*  Compare the given timestamp with the current, if it's the same must wait to the next frame */
    if(*timestamp == Kinect::temp_video_frame_timestamp)
        pthread_cond_wait(&Kinect::video_ready, &Kinect::video_lock);

    memcpy (video_frame, Kinect::temp_video_frame_raw, (VIDEO_WIDTH*VIDEO_HEIGHT)*sizeof(uint16_t));
    *timestamp = Kinect::temp_video_frame_timestamp;

    pthread_mutex_unlock(&Kinect::video_lock);
    return 0;
}

void Kinect::GetVideoFrame_ex(std::shared_ptr<KinectFrame> frame)
{
    pthread_mutex_lock(&Kinect::video_lock);

    /*  Compare the given timestamp with the current, if it's the same must wait to the next frame */
    if(frame->m_timestamp == Kinect::temp_video_frame_timestamp)
        pthread_cond_wait(&Kinect::video_ready, &Kinect::video_lock);

    frame->Fill(temp_video_frame_raw);
    frame->m_timestamp = Kinect::temp_video_frame_timestamp;

    pthread_mutex_unlock(&Kinect::video_lock);
}


void Kinect::DepthCallback(freenect_device* dev, void* data, uint32_t timestamp)
{
    pthread_mutex_lock(&Kinect::depth_lock);
    memcpy (Kinect::temp_depth_frame_raw, data, (DEPTH_WIDTH*DEPTH_HEIGHT)*sizeof(uint16_t));
    Kinect::temp_depth_frame_timestamp = timestamp;
    pthread_cond_broadcast(&Kinect::depth_ready);
    pthread_mutex_unlock(&Kinect::depth_lock);

    return;
}

void Kinect::VideoCallback(freenect_device* dev, void* data, uint32_t timestamp)
{
    pthread_mutex_lock(&Kinect::video_lock);
    memcpy (Kinect::temp_video_frame_raw, data, (VIDEO_WIDTH*VIDEO_HEIGHT)*sizeof(uint16_t));
    Kinect::temp_video_frame_timestamp = timestamp;
    pthread_cond_broadcast(&Kinect::video_ready);
    pthread_mutex_unlock(&Kinect::video_lock);

    return;
}


bool Kinect::ChangeTilt(double tilt_angle)
{
    freenect_raw_tilt_state* state;
    freenect_update_tilt_state(kinect_dev);
    int num_tries = 0;

    /* Send the desire tilt */
    if(freenect_set_tilt_degs(kinect_dev, tilt_angle))
        return true;

    /* Loop waiting */
    do
    {
        if(num_tries++ > MAX_TILT_WAIT)
            return true;

        freenect_update_tilt_state(kinect_dev);
        state = freenect_get_tilt_state(kinect_dev);
        sleep(1);
    }
    while((state->tilt_status != TILT_STATUS_STOPPED) && (state->tilt_status != TILT_STATUS_LIMIT));

    return false;
}

void *Kinect::KinectProcessEvents(void)
{
    /* Loop processing packets from kinect */
    do
    {
        if(freenect_process_events(kinect_ctx) < 0)
            return 0;

    }while(running);
    return 0;
}

void *Kinect::KinectProcessEventsHelper(void *context)
{
    return ((Kinect *)context)->KinectProcessEvents();
}

void Kinect::ChangeLedColor(freenect_led_options color)
{
    freenect_set_led(kinect_dev,color);
}

bool Kinect::IsRunning()
{
    if(running)
        return true;
    else
        return false;
}
