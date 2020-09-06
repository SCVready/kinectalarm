/**
 * @author Alejandro Solozabal
 *
 * @file alarm_component.hpp
 *
 */

#ifndef ALARM_COMPONENT_H_
#define ALARM_COMPONENT_H_
/*******************************************************************
 * Includes
 *******************************************************************/
#include <memory>
#include <thread>
#include <atomic>
#include <string>

#include "kinect.hpp"
#include "log.hpp"

/*******************************************************************
 * Class declaration
 *******************************************************************/
class AlarmComponent
{
public:
    /**
     * @brief Construct a new Liveview object
     * 
     */
    AlarmComponent(std::string component_name, std::shared_ptr<Kinect> kinect, uint32_t loop_period_ms);

    /**
     * @brief Destroy the Liveview object
     * 
     */
    ~AlarmComponent();

    /**
     * @brief 
     * 
     * @return int 
     */
    void Start();

    /**
     * @brief 
     * 
     * @return int 
     */
    void Stop();

    /**
     * @brief 
     * 
     * @return int 
     */
    bool IsRunning();

    /**
     * @brief 
     * 
     */
    void ThreadLoop();

    /**
     * @brief 
     * 
     */
    virtual void ExecutionCycle() = 0;

protected:
    std::shared_ptr<Kinect> m_kinect;
    std::unique_ptr<std::thread> m_thread;
    std::atomic<bool> m_running;
    std::string m_component_name;
    uint32_t m_loop_period_ms;
};

#endif /* ALARM_COMPONENT_H_ */