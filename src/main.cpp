/**
 * @author Alejandro Solozabal
 *
 * @file main.cpp
 *
 */

/*******************************************************************
 * Includes
 *******************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <syslog.h>
#include <pthread.h>
#include <iostream>
#include <algorithm>
#include <vector>
#include <string>
#include <cctype>
#include <sstream>
#include <iterator>

#include "global_parameters.hpp"
#include "alarm.hpp"
#include "cyclic_task.hpp"
#include "log.hpp"
#include "config.hpp"
#include "redis_db.hpp"
#include "common.hpp"
#include "message_broker_factory.hpp"

/*******************************************************************
 * Defines
 *******************************************************************/
#define KINECTALARM_VERSION "0.11"

/*******************************************************************
 * Global variables
 *******************************************************************/
volatile bool kinect_alarm_running = true;

/*******************************************************************
 * Class declaration
 *******************************************************************/
class MessageListener;

class Main : public CyclicTask
{
friend MessageListener;
public:
    Main();
    int Init();
    int Term();
private:
    void ExecutionCycle() override;

    std::shared_ptr<IMessageBroker> m_message_broker;
    std::shared_ptr<IChannelMessageObserver> m_message_observer;
    std::shared_ptr<Alarm> m_alarm;
};

class MessageListener : public IChannelMessageObserver
{
public:
    MessageListener(Main& main);
    void ChannelMessageListener(const std::string& message) override;
private:
    Main& m_main;
};

/*******************************************************************
 * Function definition
 *******************************************************************/
MessageListener::MessageListener(Main& _main) :
    m_main(_main)
{
}

void MessageListener::ChannelMessageListener(const std::string& message)
{
    std::string str(message);

    /* Remove non wanted characters */
    str.erase(std::remove_if(str.begin(), str.end(), allowed_characters), str.end());

    /* Remove consecutive spaces */
    std::string::iterator new_end = std::unique(str.begin(), str.end(), both_are_spaces);
    str.erase(new_end, str.end());

    /* Lower cases */
    std::transform(str.begin(), str.end(), str.begin(), ::tolower);

    /* Remove leading and trailing spaces */
    if(str.front() == ' ')
        str.erase(0,1);
    if(str.back() == ' ')
        str.erase(str.length()-1,1);

    std::istringstream iss(str);
    std::vector<std::string> words((std::istream_iterator<std::string>(iss)), std::istream_iterator<std::string>());

    if(words.size() >=2)
    {
        if(words[0].compare("det") == 0)
        {
            if(!words[1].compare("start"))
                m_main.m_alarm->StartDetection();
            else if(!words[1].compare("stop"))
                m_main.m_alarm->StopDetection();
            else if(!words[1].compare("rst"))
                m_main.m_alarm->ResetDetection();
            else if(!words[1].compare("del")){
                if(words.size() >=3)
                {
                    int value = std::stoi(words[2]);;
                    m_main.m_alarm->DeleteDetection(value);
                }

            }
        }
        else if(!words[0].compare("lvw"))
        {
            if(!words[1].compare("start"))
                m_main.m_alarm->StartLiveview();
            else if(!words[1].compare("stop"))
                m_main.m_alarm->StopLiveview();
        }
        else if(!words[0].compare("tilt"))
        {
            int tilt;
            try {
                tilt = std::stoi(words[1]);
            }
            catch (const std::invalid_argument& ia) {
                tilt = 0;
            }
            m_main.m_alarm->ChangeTilt(tilt);
        }
        else if(!words[0].compare("brightness"))
        {
            int value;
            try {
                value = std::stoi(words[1]);
            }
            catch (const std::invalid_argument& ia) {
                value = 0;
            }
            m_main.m_alarm->ChangeBrightness(value);
        }
        else if(!words[0].compare("contrast"))
        {
            int value;
            try {
                value = std::stoi(words[1]);
            }
            catch (const std::invalid_argument& ia) {
                value = 0;
            }
            m_main.m_alarm->ChangeContrast(value);
        }
        else if(!words[0].compare("threshold"))
        {
            int value;
            try {
                value = std::stoi(words[1]);
            }
            catch (const std::invalid_argument& ia) {
                value = 0;
            }
            m_main.m_alarm->ChangeThreshold(value);
        }
        else if(!words[0].compare("sensitivity"))
        {
            int value;
            try {
                value = std::stoi(words[1]);
            }
            catch (const std::invalid_argument& ia) {
                value = 0;
            }
            m_main.m_alarm->ChangeSensitivity(value);
        }
    }
}

Main::Main() :
    CyclicTask("Main", 1000)
{
}

int Main::Init()
{
    /* MessageBroker class creation */
    m_message_broker = MessageBrokerFactory::Create("/tmp/redis.sock");

    /* MessageListener observer creation */
    m_message_observer = std::make_shared<MessageListener>(*this);

    /* StatePersistence class creation */

    /* Alarm class creation */
    m_alarm = std::make_shared<Alarm>(m_message_broker);

    /* Alarm class initialization */
    if(0 != m_alarm->Init())
    {
        LOG(LOG_ERR, "Alarm initialization error\n");
    }

    /* Subscribe to Redis channel */
    m_message_broker->Subscribe("kinectalarm", m_message_observer);

    /* Launch watchdog thread */
    CyclicTask::Start();

    return 0;
}

int Main::Term()
{
    /* Unsubscribe to Redis channel */
    m_message_broker->Unsubscribe("kinectalarm", m_message_observer);

    /* Term watchdog thread (CyclicTask::stop) */
    CyclicTask::Stop();

    /* Alarm class term */
    m_alarm->Term();

    return 0;
}

void Main::ExecutionCycle()
{
    m_message_broker->SetVariableExpiration({"kinectalarm_watchdog",  DataType::Integer, 1}, 2); //TODO add timeout to config
}

void signalHandler(int signal)
{
    if (signal == SIGINT
        || signal == SIGTERM
        || signal == SIGQUIT)
    {
        kinect_alarm_running = false;
    }
}

int main(int argc, char** argv)
{
    /* Handle signals */
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    signal(SIGQUIT, signalHandler);

    /* Set up syslog */
    setlogmask(LOG_UPTO(LOG_DEBUG));
    openlog ("kinect_alarm", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);

#ifndef DEBUG
    LOG(LOG_NOTICE, "RELEASE BUILD %s\n", KINECTALARM_VERSION);
#else
    LOG(LOG_NOTICE, "DEBUG BUILD %s\n", KINECTALARM_VERSION);
#endif

    Main _main;
    _main.Init();

    while(kinect_alarm_running)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    _main.Term();

    return 0;
}
