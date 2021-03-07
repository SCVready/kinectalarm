/**
 * @author Alejandro Solozabal
 *
 * @file state_persistence.cpp
 *
 */

/*******************************************************************
 * Includes
 *******************************************************************/
#include <map>

#include "state_persistence.hpp"

/*******************************************************************
 * Class definition
 *******************************************************************/
Database::Database(const std::string path) :
    m_path(path)
{
    int ret_val = 0;

    ret_val = sqlite3_open(m_path.c_str(), &m_sqlite_database);

    if (ret_val != SQLITE_OK) {
        LOG(LOG_ERR,"Cannot open sqlite database on path %s. Error: %s\n", m_path.c_str(), sqlite3_errmsg(m_sqlite_database));
        sqlite3_close(m_sqlite_database);
        throw std::exception();
    }
}

Database::~Database()
{
    sqlite3_close(m_sqlite_database);
}

int Database::RemoveDatabase()
{
    std::remove(m_path.c_str());
    return 0;
}

DataTable::DataTable(std::weak_ptr<Database> data_base,const std::string& name, ListOfVariables list_variables) :
    m_name(name),
    m_data_base(data_base),
    m_list_variables(list_variables)
{
    std::shared_ptr<Database> l_data_base = m_data_base.lock();
    std::string create_command;
    char *error_message = nullptr;

    if(l_data_base == nullptr && l_data_base->m_sqlite_database == nullptr)
    {
        LOG(LOG_ERR,"Cannot create DataTable object, database is nullptr\n");
    }
    else
    {
        FormCreateTableMessage(create_command);

        /* Execute SQL statement */
        if(SQLITE_OK != sqlite3_exec(l_data_base->m_sqlite_database, create_command.c_str(), NULL, 0, &error_message))
        {
            LOG(LOG_ERR,"SQL error: %s\n", error_message);
            sqlite3_free(error_message);

            throw std::exception();
        };
    }
}

DataTable::~DataTable()
{
}

const std::map<DataType, std::string> DataTable::m_data_type_map{
    {DataType::Integer, "INTEGER"},
    {DataType::Float,   "CHAR(50)"},
    {DataType::String,  "CHAR(50)"}
};

int DataTable::FormCreateTableMessage(std::string& command)
{
    int ret_val = 0;
    /*
        "CREATE TABLE IF NOT EXISTS DETECTIONS("
        "ID             INTEGER    PRIMARY KEY,"
        "DATE           DATETIME   NOT NULL,"
        "DURATION       INTEGER    NOT NULL,"
        "FILENAME_IMG   CHAR(50)   NOT NULL,"
        "FILENAME_VID   CHAR(50)   NOT NULL);"
    */
    command = "CREATE TABLE IF NOT EXISTS " + m_name +  "(";

    for(auto it = m_list_variables.begin(); it < m_list_variables.end(); std::advance(it,1))
    {
        command += it->name + " " + m_data_type_map.at(it->data_type) + " ";

        if(it == m_list_variables.begin())
        {
            command += "PRIMARY KEY";
        }
        else
        {
            command += "NOT NULL";
        }

        if(it != std::prev(m_list_variables.end()))
        {
            command += ",";
        }
    }
    command += ");";
    return ret_val;
}

int DataTable::DeleteTable()
{
    return 0;
}