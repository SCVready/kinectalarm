/**
 * @author Alejandro Solozabal
 *
 * @file cyclic_task.cpp
 *
 */

/*******************************************************************
 * Includes
 *******************************************************************/
#include <chrono>

#include "cyclic_task.hpp"

/*******************************************************************
 * Class definition
 *******************************************************************/

CyclicTask::CyclicTask(std::string task_name, uint32_t loop_period_ms) :
    m_running(false),
    m_task_name(task_name),
    m_loop_interval_ms(loop_period_ms)
{
}

CyclicTask::~CyclicTask()
{
    Stop();
}

int CyclicTask::Start()
{
    int retval = 0;

    if(!m_running)
    {
        m_running = true;

        try
        {
            m_thread = std::make_unique<std::thread>(&CyclicTask::ThreadLoop,this);
        }
        catch(const std::exception& e)
        {
            LOG(LOG_ERR,"%s thread creation failed: %s\n", m_task_name.c_str(), e.what());
            retval = -1;
        }

        LOG(LOG_INFO,"Starting %s task\n", m_task_name.c_str());
    }
    else
    {
        LOG(LOG_INFO,"%s task is already started\n", m_task_name.c_str());
    }

    return retval;
}

int CyclicTask::Stop()
{
    int retval = 0;

    if(m_running)
    {
        m_running = false;

        m_condition_variable.notify_one();

        m_thread->join();

        LOG(LOG_INFO,"Stoping %s task\n",m_task_name.c_str());
    }
    else
    {
        LOG(LOG_INFO,"%s task is already stopped\n",m_task_name.c_str());
    }

    return retval;
}

bool CyclicTask::IsRunning()
{
    return m_running;
}

void CyclicTask::ChangeLoopInterval(uint32_t loop_interval_ms)
{
    m_loop_interval_ms = loop_interval_ms;
}

void CyclicTask::ThreadLoop()
{
    std::unique_lock<std::mutex> unique_lock(m_mutex);

    auto sleep_abs_time = std::chrono::steady_clock::now();

    while(m_running)
    {
        ExecutionCycle();
        sleep_abs_time += std::chrono::milliseconds(m_loop_interval_ms);
        m_condition_variable.wait_until(unique_lock, sleep_abs_time);
    }
}