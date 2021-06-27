/**
 * @author Alejandro Solozabal
 *
 * @file message_broker_factory.cpp
 *
 */

/*******************************************************************
 * Includes
 *******************************************************************/
#include <memory>
#include <algorithm>
#include <event2/thread.h>
#include <signal.h>

#include "message_broker.hpp"

/*******************************************************************
 * Static functions
 *******************************************************************/
static void OnMessage(redisAsyncContext *redis_context, void *replay, void *data)
{
    MessageBroker* message_broker = reinterpret_cast<MessageBroker*>(data);
    redisReply *reply = reinterpret_cast<redisReply*>(replay);
    std::string message;

    if (message_broker == nullptr)
    {
        LOG(LOG_ERR, "Redis async callback failed: message_broker null\n");
    }
    else if(reply == nullptr)
    {
        LOG(LOG_ERR, "Redis async callback failed: reply null\n");
    }
    else
    {
        /* TODO: Explain the structure of replay: "message {channel} {message}"*/

        LOG(LOG_DEBUG, "Redis replay: ");
        for (size_t i = 0; i < reply->elements; i++)
        {
            if(reply->element[i]->type == REDIS_REPLY_STRING)
            {
                LOG(LOG_DEBUG, "%s ", reply->element[i]->str);
            }
            else
            {
                LOG(LOG_DEBUG, "unk. ");
            }
        }
        LOG(LOG_DEBUG, "\n");

        /*TODO check the first string: it must be "message"*/

        if(reply->elements >= 3)
        {
            if(reply->element[1]->type == REDIS_REPLY_STRING &&
               reply->element[2]->type == REDIS_REPLY_STRING)
            {
                std::string channel(reply->element[1]->str);
                std::string messsage(reply->element[2]->str);
                if(0 != message_broker->CallObservers(channel, messsage))
                {
                    LOG(LOG_ERR, "Failed to call observers\n");
                }
            }
            else
            {
                LOG(LOG_ERR, "Replay format unknown\n");
            }
        }
    }
}

/*******************************************************************
 * Class definition
 *******************************************************************/
MessageBroker::MessageBroker(const std::string path)
{
    /* Init sync context*/
    m_context = redisConnectUnix(path.c_str());
    if(m_context == NULL || m_context->err)
    {
        if (m_context)
        {
            LOG(LOG_ERR,"redisConnectUnix() failed: %s\n", m_context->errstr);
        }
        else
        {
            LOG(LOG_ERR,"redisConnectUnix() failed: Can't allocate redis context\n");
        }
        throw std::exception();
    }

    /* Init Async context*/

    /* TODO: Received SIGPIPE */
    signal(SIGPIPE, SIG_IGN); 

    if(evthread_use_pthreads() != 0) /* This is needed to allow event_base_loopbreak() exit the loop from another thread */
    {
        LOG(LOG_ERR,"evthread_use_pthreads() failed\n");
        throw std::exception();
    }

    m_event_base = event_base_new();
    if(m_event_base == nullptr)
    {
        LOG(LOG_ERR,"event_base_new() failed\n");
        throw std::exception();
    }

    m_async_context = redisAsyncConnectUnix(path.c_str());
    if(m_async_context->err)
    {
        LOG(LOG_ERR,"redisAsyncConnectUnix() failed: %s\n", m_async_context->errstr);
        throw std::exception();
    }

    if(REDIS_OK != redisLibeventAttach(m_async_context, m_event_base))
    {
        LOG(LOG_ERR,"redisLibeventAttach() failed: %s\n", m_async_context->errstr);
        throw std::exception();
    }
}

MessageBroker::~MessageBroker()
{
    /*TODO: Unsubscribe from all the channels?*/

    if(m_proccess_async_events_thread != nullptr)
    {
        //event_base_loopexit(m_event_base, nullptr);

        event_base_loopbreak(m_event_base);

        m_proccess_async_events_thread->join();
    }

    redisFree(m_context);
}

void MessageBroker::ProccessAsyncEvents()
{
    event_base_dispatch(m_event_base);
    //event_base_loop(m_event_base, EVLOOP_NO_EXIT_ON_EMPTY);
}

int MessageBroker::Subscribe(const std::string& channel, std::shared_ptr<IChannelMessageObserver> observer)
{
    int retval = -1;
    if(observer == nullptr)
    {
        LOG(LOG_ERR,"Subscribe() failed: obs null\n");
    }
    else if(RegisterObserver(channel, observer))
    {
        LOG(LOG_ERR,"RegisterObserver() failed\n");
    }
    else if(REDIS_OK != redisAsyncCommand(m_async_context, OnMessage, reinterpret_cast<void*>(this), "SUBSCRIBE %s",channel.c_str()))
    {
        LOG(LOG_ERR,"redisAsyncCommand() failed\n");
    }
    else
    {
        if(m_proccess_async_events_thread == nullptr)
        {
            try
            {
                m_proccess_async_events_thread = std::make_unique<std::thread>(&MessageBroker::ProccessAsyncEvents,this);
                
            }
            catch(const std::exception& e)
            {
                LOG(LOG_ERR,"m_proccess_async_events_thread thread creation failed\n");
                throw std::exception();
            }
            
        }
        LOG(LOG_INFO,"Reddis subscription success: channel %s observer %p\n", channel.c_str(), observer.get());

        retval = 0;
    }

    return retval;
}

int MessageBroker::RegisterObserver(const std::string& channel, std::shared_ptr<IChannelMessageObserver> observer)
{
    int ret = 1;
    if(observer != nullptr)
    {
        auto it = m_observer_map.find(channel);
        if(it != m_observer_map.end())
        {
            /* Existing entry */
            it->second.push_back(observer);
        }
        else
        {
            /* New entry */
            auto pair = std::pair<std::string, std::vector<std::shared_ptr<IChannelMessageObserver>>>(channel,{observer});
            m_observer_map.insert(pair);
        }
        ret = 0;

        LOG(LOG_INFO,"Reddis observer registered successfully, channel %s observer %p\n", channel.c_str(), observer.get());
    }
    return ret;
}

int MessageBroker::UnRegisterObserver(const std::string& channel, std::shared_ptr<IChannelMessageObserver> observer)
{
    int ret = 1;
    if(observer != nullptr)
    {
        auto map_it = m_observer_map.find(channel);
        if(map_it != m_observer_map.end())
        {
            std::vector<std::shared_ptr<IChannelMessageObserver>>& observers = map_it->second;

            auto vec_it = std::find(observers.begin(), observers.end(), observer);
            if(vec_it != observers.end())
            {
                observers.erase(vec_it);
                if(observers.empty())
                {
                    m_observer_map.erase(map_it);
                }
                ret = 0;

                LOG(LOG_INFO,"Reddis observer unregistered successfully, channel %s observer %p\n", channel.c_str(), observer.get());
            }
        }
    }
    return ret;
}

int MessageBroker::CallObservers(const std::string& channel, const std::string& message)
{
    int ret = 1;
    auto map_it = m_observer_map.find(channel);
    if(map_it != m_observer_map.end())
    {
        for(const auto& it : map_it->second)
        {
            it->ChannelMessageListener(message);
        }
        ret = 0;
    }
    return ret;
}

int MessageBroker::Unsubscribe(const std::string& channel, const std::shared_ptr<IChannelMessageObserver> observer)
{
    int retval = 0;

    if(0 != UnRegisterObserver(channel, observer))
    {
        retval = -1;
    }

    if (m_observer_map.end() == m_observer_map.find(channel))
    {
        std::lock_guard<std::mutex> lock(m_context_mutex);

        redisReply *reply = (redisReply *) redisCommand(m_context, "UNSUBSCRIBE %s", channel.c_str());
        if(reply != nullptr && reply->type == REDIS_REPLY_ERROR)
        {
            retval = -1;
        }
        freeReplyObject(reply);
    }

    if(0 == retval)
    {
        LOG(LOG_INFO,"Reddis Unsubscription success: channel %s observer %p\n", channel.c_str(), observer.get());
    }

    return retval;
}

int MessageBroker::Publish(const std::string& channel, const std::string& message)
{
    int retval = 0;

    std::lock_guard<std::mutex> lock(m_context_mutex);

    redisReply* reply = (redisReply *) redisCommand(m_context, "PUBLISH %s %s", channel.c_str(), message.c_str());
    if(reply != nullptr && reply->type == REDIS_REPLY_ERROR)
    {
        retval = -1;
    }

    freeReplyObject(reply);

    return retval;
}

int MessageBroker::GetVariable(Variable& variable)
{
    int retval = 0;

    std::lock_guard<std::mutex> lock(m_context_mutex);

    redisReply *reply = (redisReply *) redisCommand(m_context,"GET %s",variable.name.c_str());
    if(reply->type != REDIS_REPLY_STRING)
    {
        retval = -1;
    }
    else
    {
        try
        {
            switch(variable.data_type)
            {
                case DataType::Integer:
                    variable.value = std::stoi(std::string(reply->str), nullptr);
                    break;
                case DataType::Float:
                    variable.value = std::stof(std::string(reply->str), nullptr);
                    break;
                case DataType::String:
                    variable.value = std::string(reply->str);
                    break;
                case DataType::Boolean:
                    variable.value = std::string(reply->str) == "true" ? true : false;
                    break;
            }
        }
        catch(...)
        {
            retval = -1;
        }
    }

    freeReplyObject(reply);
    return retval;
}

int MessageBroker::SetVariable(const Variable& variable)
{
    int retval = 0;

    std::string value;
    try
    {
        switch(variable.data_type)
        {
            /*TODO unify conversion to from string to value and viceversa*/
            case DataType::Integer:
            value = std::to_string(std::get<int>(variable.value));
            break;
            case DataType::Float:
            value = std::to_string(std::get<float>(variable.value));
            break;
            case DataType::String:
            value = std::get<std::string>(variable.value);
            break;
            case DataType::Boolean:
            value = std::get<bool>(variable.value) == true ? "true" : "false";
            break;
        }
        std::lock_guard<std::mutex> lock(m_context_mutex);

        redisReply *reply = (redisReply *) redisCommand(m_context,"SET %s %s", variable.name.c_str(), value.c_str());
        if(reply->type == REDIS_REPLY_ERROR)
        {
            retval = -1;
        }

        freeReplyObject(reply);
    }
    catch(...)
    {
        LOG(LOG_INFO,"Exception raised\n");
        retval = -1;
    }

    return retval;
}

int MessageBroker::SetVariableExpiration(const Variable& variable, int livetime_seconds)
{
    int retval = 0;

    std::string value;
    try
    {
        switch(variable.data_type)
        {
            case DataType::Integer:
            value = std::to_string(std::get<int>(variable.value));
            break;
            case DataType::Float:
            value = std::to_string(std::get<float>(variable.value));
            break;
            case DataType::String:
            value = std::get<std::string>(variable.value);
            break;
            case DataType::Boolean:
            value = std::get<bool>(variable.value) == true ? "true" : "false";
            break;
        }
        std::lock_guard<std::mutex> lock(m_context_mutex);

        redisReply *reply = (redisReply *) redisCommand(m_context,"SETEX %s %d %s", variable.name.c_str(), livetime_seconds, value.c_str());
        if(reply->type == REDIS_REPLY_ERROR)
        {
            retval = -1;
        }

        freeReplyObject(reply);
    }
    catch(...)
    {
        LOG(LOG_INFO,"Exception raised\n");
        retval = -1;
    }

    return retval;
}

int MessageBroker::Clear()
{
    int retval = 0;

    std::lock_guard<std::mutex> lock(m_context_mutex);

    redisReply *reply = (redisReply *) redisCommand(m_context,"flushall");
    if(reply->type != REDIS_REPLY_STRING)
    {
        retval = -1;
    }

    freeReplyObject(reply);

    return retval;
}
