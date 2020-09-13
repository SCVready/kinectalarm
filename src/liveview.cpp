/**
 * @author Alejandro Solozabal
 *
 * @file liveview.cpp
 *
 */

/*******************************************************************
 * Includes
 *******************************************************************/
#include "liveview.hpp"
#include "kinect_frame.hpp"

/*******************************************************************
 * Class definition
 *******************************************************************/

Liveview::Liveview(std::shared_ptr<Kinect> kinect, uint32_t loop_period_ms) :
    CyclicTask("Liveview", loop_period_ms),
    m_kinect(kinect)
{
    m_frame = std::make_unique<KinectFrame>(VIDEO_WIDTH,VIDEO_HEIGHT);
    m_timestamp = 0;
    liveview_jpeg = new uint8_t[VIDEO_WIDTH * VIDEO_HEIGHT * 2];

    init_base64encode(&m_c);
}

Liveview::~Liveview()
{
    deinit_base64encode(&m_c);
    delete liveview_jpeg;
}

void Liveview::ExecutionCycle()
{
    unsigned int size = 0;

    m_kinect->GetVideoFrame_ex(m_frame);
    LOG(LOG_INFO,"Liveview cycle: frame taken\n");

    /*TODO: Move to alarm class in a callback */
    /* Convert to jpeg */
    save_video_frame_to_jpeg_inmemory(m_frame->GetDataPointer(), liveview_jpeg,&size,200,0);

    /* Convert to base64 */
    char *base64_encoded = base64encode(&m_c, liveview_jpeg, size);

    /* Publish in redis channel */
    redis_publish("liveview", base64_encoded);
}