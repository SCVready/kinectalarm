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
    DataTable(std::weak_ptr<Database> data_base, const std::string& name, ListOfVariables list_variables);
    ~DataTable();
    int NumberItems();
    int InsertItem();
    int GetItem();
    int SetItem();
    int DeleteItem();
    int DeleteAllItems();
    int DeleteTable();
private:
    const std::string m_name;
    std::weak_ptr<Database> m_data_base;
    ListOfVariables m_list_variables;

    const static std::map<DataType, std::string> m_data_type_map;

    int FormCreateTableMessage(std::string& command);
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
