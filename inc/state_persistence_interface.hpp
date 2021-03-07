/**
 * @author Alejandro Solozabal
 *
 * @file state_persistence_interface.hpp
 *
 */

#ifndef ISTATE_PERSISTANCE__H_
#define ISTATE_PERSISTANCE__H_

/*******************************************************************
 * Includes
 *******************************************************************/
#include <string>
#include <vector>
#include <tuple>
#include <variant>

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

/* */
struct Variable
{
    const std::string name;
    const DataType data_type;
    Value value;
};

/* */
using ListOfVariables = std::vector<Variable>;

/* Initialization parameters */
struct InitParams{};

/*******************************************************************
 * Class declaration
 *******************************************************************/
class IStatePersistence
{
public:
    /**
     * @brief Destructor
     * 
     */
    virtual ~IStatePersistence() {};

    virtual int RemoveDatabase() = 0;
};

#endif /* ISTATE_PERSISTANCE__H_ */
