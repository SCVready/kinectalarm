/**
 * @author Alejandro Solozabal
 *
 * @file liveview.hpp
 *
 */

#ifndef LIVEVIEW_H_
#define LIVEVIEW_H_

/*******************************************************************
 * Includes
 *******************************************************************/
#include <memory>

#include "common.hpp"
#include "global_parameters.hpp"
#include "log.hpp"
#include "kinect.hpp"
#include "cyclic_task.hpp"
#include "jpeg.hpp"


/*******************************************************************
 * Class declaration
 *******************************************************************/
class LiveviewObserver
{
public:
    virtual void NewFrame(char* base64_jpeg_frame) = 0;
};

class Liveview : public CyclicTask
{
public:
    /**
     * @brief Construct a new Liveview object
     * 
     */
    Liveview(std::shared_ptr<Kinect> kinect, std::shared_ptr<LiveviewObserver> liveview_observer, uint32_t loop_period_ms);

    /**
     * @brief Destroy the Liveview object
     * 
     */
    ~Liveview();

    void ExecutionCycle() override;

private:
    std::shared_ptr<Kinect> m_kinect;
    std::shared_ptr<KinectFrame> m_frame;
    struct sBase64encode_context m_c;
    std::shared_ptr<LiveviewObserver> m_liveview_observer;
};

#endif /* LIVEVIEW_H_ */