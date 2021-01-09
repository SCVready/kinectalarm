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
#include <condition_variable>
#include <mutex>

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
    CyclicTask(std::string task_name, uint32_t loop_interval_ms);

    /**
     * @brief Destructor
     * 
     */
    ~CyclicTask();

    /**
     * @brief Start cyclic task
     * 
     * @return int
     */
    int Start();

    /**
     * @brief Stop cyclic task
     * 
     * @return int 
     */
    int Stop();

    /**
     * @brief Check if the cyclic task is running
     * 
     * @return int 
     */
    bool IsRunning();

    /**
     * @brief Change the loop interval of the task
     * 
     */
    void ChangeLoopInterval(uint32_t loop_interval_ms);

private:
    std::unique_ptr<std::thread> m_thread;
    std::atomic<bool> m_running;
    std::string m_task_name;
    std::atomic<uint32_t> m_loop_interval_ms;
    std::mutex m_mutex;
    std::condition_variable m_condition_variable;

    /* Private funtions */
    void ThreadLoop();
    virtual void ExecutionCycle() = 0;
};

#endif /* CYCLIC_TASK_H_ */