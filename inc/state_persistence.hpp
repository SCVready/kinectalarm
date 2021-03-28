/**
 * @author Alejandro Solozabal
 *
 * @file state_persistence.hpp
 *
 */

#ifndef STATE_PERSISTANCE__H_
#define STATE_PERSISTANCE__H_

/*******************************************************************
 * Includes
 *******************************************************************/
#include <memory>
#include <sqlite3.h>

#include "state_persistence_interface.hpp"
#include "log.hpp"

/*******************************************************************
 * Class declaration
 *******************************************************************/
class Database;
class DataTable
{
public:
    DataTable(std::weak_ptr<Database> data_base, const std::string& name, Entry list_variables);
    ~DataTable();
    int NumberItems(int& number_items);
    int InsertItem(const Entry& item);
    int GetItem(Entry& item);
    int SetItem();
    int DeleteItem();
    int DeleteAllItems();
    int DeleteTable();
private:
    const std::string m_name;
    std::weak_ptr<Database> m_data_base;
    Entry m_list_variables;

    const static std::map<DataType, std::string> m_data_type_map;

    int ExecuteSqlCommand(const std::string& command);
    int ExecuteSqlRequest(const std::string& command, sqlite3_stmt **response);

    int FormCreateTableMessage(std::string& command);
    int FormDeleteTableMessage(std::string& command);
    int FormNumberItemsMessage(std::string& command);
    int FormInsertItemMessage(std::string& command, const Entry& item);
    int FormGetItemMessage(std::string& command, const Entry& item);

    int HandleNumberItemsResponse(sqlite3_stmt **response, int& number_items);
    int HandleGetItemResponse(sqlite3_stmt **response, Entry& item);

    std::string VariableToString(const Variable& variable);
    int StringToVariable(const std::string& string_value, Variable& variable);
};

class Database : public IStatePersistence
{
    friend DataTable;
public:
    Database() = delete;

    Database(const std::string path) noexcept(false);

    int RemoveDatabase();

    ~Database();
private:
    const std::string m_path;
    sqlite3 *m_sqlite_database;
};

#endif /* STATE_PERSISTANCE__H_ */
