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
using Entry = std::vector<Variable>;

/* Initialization parameters */
struct InitParams{};

/*******************************************************************
 * Class declaration
 *******************************************************************/
class IDataTable
{
public:
    /**
     * @brief Destructor
     * 
     */
    virtual ~IDataTable() {};

    virtual int NumberItems(int& number_items) = 0;
    virtual int InsertItem(const Entry& item) = 0;
    virtual int GetItem(Entry& item) = 0;
    virtual int SetItem(const Entry& item) = 0;
    virtual int DeleteItem(const Entry& item) = 0;
    virtual int DeleteAllItems() = 0;
    virtual int DeleteTable() = 0;
};

class IDatabase
{
public:
    /**
     * @brief Destructor
     * 
     */
    virtual ~IDatabase() {};

    virtual int RemoveDatabase() = 0;
};

#endif /* ISTATE_PERSISTANCE__H_ */
