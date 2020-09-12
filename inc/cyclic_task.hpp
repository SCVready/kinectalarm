/**
 * @author Alejandro Solozabal
 *
 * @file cyclic_task.hpp
 *
 */

#ifndef CYCLIC_TASK_H_
#define CYCLIC_TASK_H_
/*******************************************************************
 * Includes
 *******************************************************************/
#include <memory>
#include <thread>
#include <atomic>
#include <string>

#include "log.hpp"

/*******************************************************************
 * Class declaration
 *******************************************************************/
class CyclicTask
{
public:
    /**
     * @brief Construct a new Liveview object
     * 
     */
    CyclicTask(std::string task_name, uint32_t loop_period_ms);

    /**
     * @brief Destroy the Liveview object
     * 
     */
    ~CyclicTask();

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
    std::unique_ptr<std::thread> m_thread;
    std::atomic<bool> m_running;
    std::string m_task_name;
    uint32_t m_loop_period_ms;
};

#endif /* CYCLIC_TASK_H_ */