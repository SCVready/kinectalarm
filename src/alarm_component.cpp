/**
 * @author Alejandro Solozabal
 *
 * @file alarm_component.cpp
 *
 */

/*******************************************************************
 * Includes
 *******************************************************************/
#include "alarm_component.hpp"

/*******************************************************************
 * Class definition
 *******************************************************************/

AlarmComponent::AlarmComponent(std::string component_name, std::shared_ptr<Kinect> kinect, uint32_t loop_period_ms) :
    m_kinect(kinect),
    m_running(false),
    m_component_name(component_name),
    m_loop_period_ms(loop_period_ms)
{
    ;
}

AlarmComponent::~AlarmComponent()
{
    ;
}

void AlarmComponent::Start()
{
    if(!m_running)
    {
        m_running = true;

        m_thread = std::make_unique<std::thread>(&AlarmComponent::ThreadLoop,this);

        LOG(LOG_INFO,"Starting %s thread\n",m_component_name.c_str());
    }
    else
    {
        LOG(LOG_INFO,"%s thread is already started\n",m_component_name.c_str());
    }
}

void AlarmComponent::Stop()
{
    if(m_running)
    {
        m_running = false;

        m_thread->join();

        LOG(LOG_INFO,"Stoping %s thread\n",m_component_name.c_str());
    }
    else
    {
        LOG(LOG_INFO,"%s thread is already stoped\n",m_component_name.c_str());
    }
}

bool AlarmComponent::IsRunning()
{
    return m_running;
}

void AlarmComponent::ThreadLoop()
{
    auto sleep_abs_time = std::chrono::system_clock::now();
    while(m_running)
    {
        ExecutionCycle();
        sleep_abs_time += std::chrono::milliseconds(m_loop_period_ms);
        std::this_thread::sleep_until(sleep_abs_time);
    }
}