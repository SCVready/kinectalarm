#include "message_broker_mock.hpp"

template<typename T, typename... Ts>
std::ostream& operator<<(std::ostream& os, const std::variant<T, Ts...>& v)
{
    std::visit([&os](auto&& arg){
        os << arg;
    }, v);
    return os;
}

std::ostream& operator<<(std::ostream& os, const DataType& v)
{
    switch (v)
    {
        case DataType::Integer: return (os << "Integer");
        case DataType::Float:   return (os << "Float");
        case DataType::String:  return (os << "String");
        case DataType::Boolean: return (os << "Boolean");
    }

    return os;
}

std::ostream& operator<<(std::ostream& os, const Variable& v)
{
    os << "{" << v.name << ", " <<  v.data_type  << ", " << v.value;

    return os;
}

MessageBrokerMock::MessageBrokerMock()
{
}

MessageBrokerMock::~MessageBrokerMock()
{
}
