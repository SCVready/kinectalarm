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

#include "message_broker.hpp"

/*******************************************************************
 * Static functions
 *******************************************************************/
void OnMessage(redisAsyncContext *redis_context, void *replay, void *data)
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
        for (int i = 0; i < reply->elements; i++)
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

    //signal(SIGPIPE, SIG_IGN); /* TODO: Check if needed */

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

        Publish(channel,{}); /*TODO: this is a dummy message. Investigate why the first message is not sent*/

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
    return 0;
}
int MessageBroker::Publish(const std::string& channel, const std::string& message)
{
    int retval = 0;

    redisReply *reply = nullptr;

    reply = (redisReply *) redisCommand(m_context, "PUBLISH %s %s", channel.c_str(), message.c_str());
    if(reply != nullptr && reply->type == REDIS_REPLY_ERROR)
    {
        retval = -1;
    }

    freeReplyObject(reply);

    return retval;
}
int MessageBroker::GetVariable(Variable& variable)
{
    return 0;
}

int MessageBroker::SetVariable(const Variable& variable)
{
    return 0;
}
