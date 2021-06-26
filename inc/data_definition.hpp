/**
 * @author Alejandro Solozabal
 *
 * @file data_definition.hpp
 *
 */

#ifndef DATA_DEFINITION__H_
#define DATA_DEFINITION__H_

/*******************************************************************
 * Includes
 *******************************************************************/
#include <string>
#include <vector>
#include <tuple>
#include <variant>

/*******************************************************************
 * Definitions
 *******************************************************************/
enum class DataType
{
    Integer,
    Float,
    String,
    Boolean
};

/* */
using Value = std::variant<int32_t, float, std::string, bool>;

/* */
struct Variable
{
    std::string name;
    DataType data_type;
    Value value;
};

/* */
using Entry = std::vector<Variable>;

#endif /* DATA_DEFINITION__H_ */