/**
 * @author Alejandro Solozabal
 *
 * @file cyclic_task.cpp
 *
 */

/*******************************************************************
 * Includes
 *******************************************************************/
#include "cyclic_task.hpp"

/*******************************************************************
 * Class definition
 *******************************************************************/

CyclicTask::CyclicTask(std::string task_name, uint32_t loop_period_ms) :
    m_running(false),
    m_task_name(task_name),
    m_loop_period_ms(loop_period_ms)
{
    ;
}

CyclicTask::~CyclicTask()
{
    ;
}

void CyclicTask::Start()
{
    //TODO catch exceptions, return value
    if(!m_running)
    {
        m_running = true;

        m_thread = std::make_unique<std::thread>(&CyclicTask::ThreadLoop,this);

        LOG(LOG_INFO,"Starting %s thread\n",m_task_name.c_str());
    }
    else
    {
        LOG(LOG_INFO,"%s thread is already started\n",m_task_name.c_str());
    }
}

void CyclicTask::Stop()
{
    if(m_running)
    {
        m_running = false;

        m_thread->join();

        LOG(LOG_INFO,"Stoping %s thread\n",m_task_name.c_str());
    }
    else
    {
        LOG(LOG_INFO,"%s thread is already stoped\n",m_task_name.c_str());
    }
}

bool CyclicTask::IsRunning()
{
    return m_running;
}

void CyclicTask::ThreadLoop()
{
    auto sleep_abs_time = std::chrono::system_clock::now();
    while(m_running)
    {
        ExecutionCycle();
        sleep_abs_time += std::chrono::milliseconds(m_loop_period_ms);
        std::this_thread::sleep_until(sleep_abs_time);
        /* TODO: Sleep with conditional variable */
    }
}