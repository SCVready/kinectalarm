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

#include "kinect.hpp"
#include "cyclic_task.hpp"
#include "log.hpp"
#include "global_parameters.hpp"
#include "jpeg.hpp"
#include "common.hpp"
#include "redis_db.hpp"

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
    std::shared_ptr<KinectFrame> m_frame;
    uint32_t m_timestamp;
    std::shared_ptr<Kinect> m_kinect;
    struct sBase64encode_context m_c;
    uint8_t* liveview_jpeg;
};

#endif /* LIVEVIEW_H_ */