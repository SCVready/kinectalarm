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
     * @brief Constructor
     * 
     */
    CyclicTask(std::string task_name, uint32_t loop_period_ms);

    /**
     * @brief Destructor
     * 
     */
    ~CyclicTask();

    /**
     * @brief Start cyclic task
     * 
     * @return int TODO
     */
    void Start();

    /**
     * @brief Stop cyclic task
     * 
     * @return int 
     */
    void Stop();

    /**
     * @brief Check if the cyclic task is running
     * 
     * @return int 
     */
    bool IsRunning();

private:
    std::unique_ptr<std::thread> m_thread;
    std::atomic<bool> m_running;
    std::string m_task_name;
    uint32_t m_loop_period_ms;

    /* Private funtions */
    void ThreadLoop();
    virtual void ExecutionCycle() = 0;
};

#endif /* CYCLIC_TASK_H_ */