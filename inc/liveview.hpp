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
#include "alarm_component.hpp"
#include "log.hpp"
#include "global_parameters.hpp"

/*******************************************************************
 * Class declaration
 *******************************************************************/
class Liveview : public AlarmComponent
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
    uint32_t m_timestamp;

};

#endif /* LIVEVIEW_H_ */