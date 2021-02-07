/**
 * @author Alejandro Solozabal
 *
 * @file message_broker_interface.hpp
 *
 */

#ifndef IMESSAGE_BROKER__H_
#define IMESSAGE_BROKER__H_

/*******************************************************************
 * Includes
 *******************************************************************/
#include <string>
#include <vector>
#include <tuple>
#include <variant>
#include <memory>

/*******************************************************************
 * Enumeration
 *******************************************************************/
enum class DataType
{
    Integer,
    Float,
    String
};

/* */
using Value = std::variant<int32_t, float, std::string>;

/* Name, type and value */
using Variable = std::tuple<std::string, DataType, Value>;

/* */
using ListOfVariables = std::vector<Variable>;

/*******************************************************************
 * Class declaration
 *******************************************************************/
class IChannelMessageObserver
{
public:
    virtual void ChannelMessageListener(const std::string& message) = 0;
};

class IMessageBroker
{
public:
    virtual ~IMessageBroker() {};

    /**
     * @brief Subscribe
     * 
     * @return 0 if ok
     */
    virtual int Subscribe(const std::string& channel, const std::shared_ptr<IChannelMessageObserver> observer) = 0;

    /**
     * @brief UnSubscribe
     * 
     * @return 0 if ok
     */
    virtual int Unsubscribe(const std::string& channel, const std::shared_ptr<IChannelMessageObserver> observer) = 0;

    /**
     * @brief Publish
     * 
     * @return 0 if ok
     */
    virtual int Publish(const std::string& channel, const std::string& message) = 0;

    /**
     * @brief GetVariable
     * 
     * @return 0 if ok
     */
    virtual int GetVariable(Variable& variable) = 0;

    /**
     * @brief SetVariable
     * 
     * @return 0 if ok
     */
    virtual int SetVariable(const Variable& variable) = 0;
};

#endif /* IMESSAGE_BROKER__H_ */
