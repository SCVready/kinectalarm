/**
 * @author Alejandro Solozabal
 *
 * @file message_broker_factory_fake.cpp
 *
 */

/*******************************************************************
 * Includes
 *******************************************************************/
#include <memory>

#include "../../../inc/message_broker_factory.hpp"

/*******************************************************************
 * Class definition
 *******************************************************************/
extern std::shared_ptr<IMessageBroker> message_broker_mock;

std::shared_ptr<IMessageBroker> MessageBrokerFactory::Create(std::string path)
{
    return nullptr;
}
