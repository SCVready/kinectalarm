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

Liveview::Liveview(std::shared_ptr<Kinect> kinect, std::shared_ptr<LiveviewObserver> liveview_observer, uint32_t loop_period_ms) :
    CyclicTask("Liveview", loop_period_ms),
    m_kinect(kinect),
    m_liveview_observer(liveview_observer)
{
    m_frame = std::make_unique<KinectFrame>(VIDEO_WIDTH,VIDEO_HEIGHT);

    init_base64encode(&m_c);
}

Liveview::~Liveview()
{
    deinit_base64encode(&m_c);
}

void Liveview::ExecutionCycle()
{
    static std::vector<uint8_t> liveview_jpeg;

    m_kinect->GetVideoFrame_ex(m_frame);
    LOG(LOG_INFO,"Liveview cycle: frame taken\n");

    /* Convert to jpeg */
    save_video_frame_to_jpeg_inmemory(m_frame->GetDataPointer(), liveview_jpeg, 200, 0);

    /* Convert to base64 */
    char *base64_encoded = base64encode(&m_c, liveview_jpeg.data(), liveview_jpeg.size());

    m_liveview_observer->NewFrame(base64_encoded);
}