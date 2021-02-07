/**
 * @author Alejandro Solozabal
 *
 * @file message_broker_factory.hpp
 *
 */

#ifndef MESSAGE_BROKER_FACTORY__H_
#define MESSAGE_BROKER_FACTORY__H_

/*******************************************************************
 * Includes
 *******************************************************************/
#include <memory>

#include "message_broker_interface.hpp"

/*******************************************************************
 * Class declaration
 *******************************************************************/
class MessageBrokerFactory
{
public:
    static std::shared_ptr<IMessageBroker> Create();
};

#endif /* MESSAGE_BROKER_FACTORY__H_ */