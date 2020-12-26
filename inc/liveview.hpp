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

/*******************************************************************
 * Class declaration
 *******************************************************************/
class LiveviewObserver
{
public:
    virtual void NewFrame(std::shared_ptr<KinectVideoFrame> frame) = 0;
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
    std::shared_ptr<KinectVideoFrame> m_frame;
    std::shared_ptr<LiveviewObserver> m_liveview_observer;
};

#endif /* LIVEVIEW_H_ */