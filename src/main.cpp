/**
 * @author Alejandro Solozabal
 *
 * @file main.cpp
 *
 */

/*******************************************************************
 * Includes
 *******************************************************************/
#include <signal.h>
#include <syslog.h>
#include <algorithm>
#include <vector>
#include <string>
#include <sstream>
#include <iterator>

#include "global_parameters.hpp"
#include "alarm.hpp"
#include "cyclic_task.hpp"
#include "log.hpp"
#include "common.hpp"
#include "message_broker_factory.hpp"
#include "state_persistence_factory.hpp"

/*******************************************************************
 * Global variables
 *******************************************************************/
volatile bool kinectalarm_running = true;

enum class Target
{
    Detection,
    Liveview,
    Tilt,
    Brightness,
    Contrast,
    Threshold,
    Sensitivity
};

enum class Action
{
    Start,
    Stop,
    Reset,
    Delete
};

const std::map<std::string, Target> parameter_map
{
    {"det",         Target::Detection},
    {"lvw",         Target::Liveview},
    {"tilt",        Target::Tilt},
    {"brightness",  Target::Brightness},
    {"contrast",    Target::Contrast},
    {"threshold",   Target::Threshold},
    {"sensitivity", Target::Sensitivity},
};

const std::map<std::string, Action> action_map
{
    {"start", Action::Start},
    {"stop",  Action::Stop},
    {"rst",   Action::Reset},
    {"del",   Action::Delete},
};

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
    std::shared_ptr<IDatabase> m_data_base;
    std::shared_ptr<Alarm> m_alarm;
};

class MessageListener : public IChannelMessageObserver
{
public:
    MessageListener(Main& main);
    void ChannelMessageListener(const std::string& message) override;
private:
    Main& m_main;
    void ParseCommandWords(const std::string& message, std::vector<std::string>& command_words);
};

/*******************************************************************
 * Function definition
 *******************************************************************/
Main::Main() :
    CyclicTask("Watchdog", WATCHDOG_REFRESH_MS)
{
    /* Create base directory to save detection images */
    if(0 != CreateDirectory(DETECTION_PATH))
    {
        LOG(LOG_ERR, "Error creating Detection directory: %s\n", DETECTION_PATH);
        throw std::exception();
    }
    /* Create Redis DB object */
    else if(nullptr == (m_message_broker = MessageBrokerFactory::Create(REDIS_DB_PATH)))
    {
        LOG(LOG_ERR, "Error creating MessageBroker object on path: %s\n", REDIS_DB_PATH);
        throw std::exception();
    }
    /* Create SQLite DB object */
    else if(nullptr == (m_data_base = StatePersistenceFactory::CreateDatabase(SQLITE_DB_PATH)))
    {
        LOG(LOG_ERR, "Error creating StatePersistence object on path: %s\n", SQLITE_DB_PATH);
        throw std::exception();
    }

    /* MessageListener observer creation */
    m_message_observer = std::make_shared<MessageListener>(*this);

    /* Alarm class creation */
    m_alarm = std::make_shared<Alarm>(m_message_broker, m_data_base);
}

int Main::Init()
{
    int ret_val = -1;

    /* Set version on Redis */
    if(0 != (m_message_broker->SetVariable({"kinectalarm_version",  DataType::String, std::string(KINECTALARM_VERSION)})))
    {
        LOG(LOG_ERR, "Error setting kinectalarm version on Redis\n");
    }
    /* Alarm class initialization */
    else if(0 != m_alarm->Init())
    {
        LOG(LOG_ERR, "Alarm initialization error\n");
    }
    /* Subscribe to the Redis channel where the commands will be published */
    else if(0 != m_message_broker->Subscribe(REDIS_COMMAND_CHANNEL, m_message_observer))
    {
        LOG(LOG_ERR, "Error trying to subscribe to command Redis channel: %s\n", REDIS_COMMAND_CHANNEL);
    }
    /* Launch watchdog task */
    else if(0 != CyclicTask::Start())
    {
        LOG(LOG_ERR, "Error launching the watchdog task\n");
    }
    else
    {
        LOG(LOG_INFO, "Main initialization success\n");
        ret_val = 0;
    }

    return ret_val;
}

int Main::Term()
{
    int ret_val = 0;

    /* Unsubscribe to Redis channel */
    if(m_message_broker != nullptr)
    {
        if(0 != m_message_broker->Unsubscribe(REDIS_COMMAND_CHANNEL, m_message_observer))
        {
            LOG(LOG_ERR, "Error trying to unsubscribe from command Redis channel: %s\n", REDIS_COMMAND_CHANNEL);
            ret_val = -1;
        }
    }

    /* Alarm class term */
    if(m_alarm != nullptr)
    {
        if(0 != m_alarm->Term())
        {
            LOG(LOG_ERR, "Alarm termination error\n");
            ret_val = -1;
        }
    }

    /* Launch watchdog task */
    if(0 != CyclicTask::Stop())
    {
        LOG(LOG_ERR, "Error stopping the watchdog task\n");
        ret_val = -1;
    }


    return ret_val;
}

void Main::ExecutionCycle()
{
    m_message_broker->SetVariableExpiration({"kinectalarm_watchdog",  DataType::Integer, 1}, WATCHDOG_TIMEOUT_S);
}

void signalHandler(int signal)
{
    if(signal == SIGINT  ||
       signal == SIGTERM ||
       signal == SIGQUIT)
    {
        kinectalarm_running = false;
    }
}

int main(int argc, char** argv)
{
    /* Handle signals */
    signal(SIGINT,  signalHandler);
    signal(SIGTERM, signalHandler);
    signal(SIGQUIT, signalHandler);

#ifndef DEBUG
    LOG(LOG_NOTICE, "RELEASE BUILD %s\n", KINECTALARM_VERSION);
    setlogmask(LOG_UPTO(LOG_INFO));
#else
    LOG(LOG_NOTICE, "DEBUG BUILD %s\n", KINECTALARM_VERSION);
    setlogmask(LOG_UPTO(LOG_DEBUG));
#endif

    /* Set up syslog */
    openlog("kinectalarm", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);

    try
    {
        Main _main;

        if(0 == _main.Init())
        {
            while(kinectalarm_running)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }

        _main.Term();
    }
    catch(const std::exception& e)
    {
        LOG(LOG_ERR,"Exception :%s\n", e.what());
    }

    return 0;
}

MessageListener::MessageListener(Main& _main) :
    m_main(_main)
{
}

void MessageListener::ParseCommandWords(const std::string& message, std::vector<std::string>& command_words)
{
    std::string command(message);

    /* Remove non wanted characters */
    command.erase(std::remove_if(command.begin(), command.end(), AllowedCharacters), command.end());

    /* Remove consecutive spaces */
    std::string::iterator new_end = std::unique(command.begin(), command.end(), BothAreSpaces);
    command.erase(new_end, command.end());

    /* Lower cases */
    std::transform(command.begin(), command.end(), command.begin(), ::tolower);

    /* Remove leading and trailing spaces */
    if(command.front() == ' ')
    {
        command.erase(0,1);
    }
    if(command.back() == ' ')
    {
        command.erase(command.length()-1,1);
    }

    std::istringstream input_string_stream(command);
    std::vector<std::string> words((std::istream_iterator<std::string>(input_string_stream)), std::istream_iterator<std::string>());
    command_words = words;
}

void MessageListener::ChannelMessageListener(const std::string& message)
{

    std::vector<std::string> command_words;
    Target parameter = Target::Detection;
    Action action = Action::Start;
    int value = 0;

    ParseCommandWords(message, command_words);

    try
    {
        parameter = parameter_map.at(command_words.at(0));
        switch(parameter)
        {
            case Target::Detection:
                action = action_map.at(command_words.at(1));
                switch(action)
                {
                    case Action::Start:
                        m_main.m_alarm->StartDetection();
                        break;
                    case Action::Stop:
                        m_main.m_alarm->StopDetection();
                        break;
                    case Action::Reset:
                        m_main.m_alarm->ResetDetection();
                        break;
                    case Action::Delete:
                        value = std::stoi(command_words.at(2));;
                        m_main.m_alarm->DeleteDetection(value);
                        break;
                    default:
                        break;
                }
                break;
            case Target::Liveview:
                action = action_map.at(command_words.at(1));
                switch(action)
                {
                    case Action::Start:
                        m_main.m_alarm->StartLiveview();
                        break;
                    case Action::Stop:
                        m_main.m_alarm->StopLiveview();
                        break;
                    default:
                        break;
                }
                break;
            case Target::Tilt:
                value = std::stoi(command_words.at(1));
                m_main.m_alarm->ChangeTilt(value);
                break;
            case Target::Brightness:
                value = std::stoi(command_words.at(1));
                m_main.m_alarm->ChangeBrightness(value);
                break;
            case Target::Contrast:
                value = std::stoi(command_words.at(1));
                m_main.m_alarm->ChangeContrast(value);
                break;
            case Target::Threshold:
                value = std::stoi(command_words.at(1));
                m_main.m_alarm->ChangeThreshold(value);
                break;
            case Target::Sensitivity:
                value = std::stoi(command_words.at(1));
                m_main.m_alarm->ChangeSensitivity(value);
                break;
            default:
                break;
        }
    }
    catch(const std::exception& e)
    {
        LOG(LOG_ERR,"Exception parsing command :%s\n", e.what());
    }
}