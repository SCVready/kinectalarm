/**
 * @author Alejandro Solozabal
 *
 * @file message_broker.hpp
 *
 */

#ifndef MESSAGE_BROKER__H_
#define MESSAGE_BROKER__H_

/*******************************************************************
 * Includes
 *******************************************************************/
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <thread>
#include <mutex>

#include <hiredis/hiredis.h>
#include <hiredis/async.h>
#include <hiredis/adapters/libevent.h>

#include "message_broker_interface.hpp"
#include "log.hpp"

/*******************************************************************
 * Class declaration
 *******************************************************************/
/*TODO: check evsignal_new  https://github.com/libevent/libevent/blob/master/sample/hello-world.c*/


class ChannelMessageObserver : public IChannelMessageObserver
{
public:
    void ChannelMessageListener(const std::string& message) override {};
};

class MessageBroker : public IMessageBroker
{
public:
    MessageBroker() = delete;

    MessageBroker(const std::string path) noexcept(false);

    ~MessageBroker();

    int Subscribe(const std::string& channel, const std::shared_ptr<IChannelMessageObserver> observer) override;

    int Unsubscribe(const std::string& channel, const std::shared_ptr<IChannelMessageObserver> observer) override;

    int Publish(const std::string& channel, const std::string& message) override;

    int GetVariable(Variable& variable) override;

    int SetVariable(const Variable& variable) override;

    int SetVariableExpiration(const Variable& variable, int livetime_seconds) override;

    int Clear() override;

    int CallObservers(const std::string& channel, const std::string& message);
private:
    redisContext *m_context = nullptr;
    redisAsyncContext *m_async_context = nullptr;
    event_base *m_event_base = nullptr;
    std::mutex m_context_mutex;
    std::unique_ptr<std::thread> m_proccess_async_events_thread;
    std::map<std::string, std::vector<std::shared_ptr<IChannelMessageObserver>>> m_observer_map;

    void ProccessAsyncEvents();
    int RegisterObserver(const std::string& channel, std::shared_ptr<IChannelMessageObserver> observer);
    int UnRegisterObserver(const std::string& channel, std::shared_ptr<IChannelMessageObserver> observer);
};

#endif /* MESSAGE_BROKER__H_ */
