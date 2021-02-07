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

#include "message_broker_factory.hpp"
#include "message_broker.hpp"

/*******************************************************************
 * Class definition
 *******************************************************************/

std::shared_ptr<IMessageBroker> MessageBrokerFactory::Create()
{
    return std::make_shared<MessageBroker>();
}
