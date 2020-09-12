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
#include <thread>
#include <atomic>

#include "kinect.hpp"
#include "cyclic_task.hpp"
#include "log.hpp"
#include "global_parameters.hpp"

/*******************************************************************
 * Class declaration
 *******************************************************************/
class Liveview : public CyclicTask
{
public:
    /**
     * @brief Construct a new Liveview object
     * 
     */
    Liveview(std::shared_ptr<Kinect> kinect, uint32_t loop_period_ms);

    /**
     * @brief Destroy the Liveview object
     * 
     */
    ~Liveview();

    void ExecutionCycle() override;

private:
    uint16_t* m_frame;
    std::shared_ptr<KinectFrame> m_frame_ex;
    uint32_t m_timestamp;
    std::shared_ptr<Kinect> m_kinect;
};

#endif /* LIVEVIEW_H_ */