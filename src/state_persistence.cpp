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
const std::map<DataType, std::string> DataTable::m_data_type_map{
    {DataType::Integer, "INTEGER"},
    {DataType::Float,   "CHAR(50)"},
    {DataType::String,  "CHAR(50)"},
    {DataType::Boolean, "CHAR(50)"}
};

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

DataTable::DataTable(std::weak_ptr<IDatabase> data_base, const std::string& name, Entry list_variables) :
    m_name(name),
    m_list_variables(list_variables)
{
    std::string command;

    m_data_base = std::weak_ptr<Database>(std::dynamic_pointer_cast<Database>(data_base.lock()));

    if(0 != FormCreateTableMessage(command))
    {
        LOG(LOG_ERR,"Error forming CreateTable message\n");
    }
    else if(0 != ExecuteSqlCommand(command))
    {
        LOG(LOG_ERR,"Failed to create table\n");
        throw std::exception();
    }
}

DataTable::~DataTable()
{
}

int DataTable::FormCreateTableMessage(std::string& command)
{
    int ret_val = 0;

    /*
     *  CREATE TABLE IF NOT EXISTS tablename(
     *     ID             INTEGER    PRIMARY KEY,
     *     DATE           DATETIME   NOT NULL,
     *     DURATION       INTEGER    NOT NULL,
     *     FILENAME_IMG   CHAR(50)   NOT NULL,
     *     FILENAME_VID   CHAR(50)   NOT NULL);
     */
    command = "CREATE TABLE IF NOT EXISTS " + m_name +  "(";

    for(auto it = m_list_variables.begin(); it != m_list_variables.end(); std::advance(it,1))
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
    int ret_val = 0;
    std::string command;
    if(0 != FormDeleteTableMessage(command))
    {
        LOG(LOG_ERR,"Error forming DeleteTable message\n");
    }
    else if(0 != ExecuteSqlCommand(command))
    {
        LOG(LOG_ERR,"Failed to delete table\n");
        ret_val = -1;
    }

    return ret_val;
}

int DataTable::FormDeleteTableMessage(std::string& command)
{
    int ret_val = 0;

    /*
     * DROP TABLE IF EXISTS tablename;
     */

    command = "DROP TABLE IF EXISTS " + m_name ;

    return ret_val;
}

int DataTable::ExecuteSqlCommand(const std::string& command)
{
    int ret_val = 0;
    char *error_message = nullptr;
    std::shared_ptr<Database> l_data_base = m_data_base.lock();

    if(l_data_base == nullptr && l_data_base->m_sqlite_database == nullptr)
    {
        LOG(LOG_ERR,"ExecuteSqlCommand failed, database is nullptr\n");
    }
    else
    {
        /* Execute SQL statement */
        if(SQLITE_OK != sqlite3_exec(l_data_base->m_sqlite_database, command.c_str(), NULL, 0, &error_message))
        {
            LOG(LOG_ERR,"SQL command error: %s\n", error_message);
            sqlite3_free(error_message);

            ret_val = 1;
        };
    }

    return ret_val;
}

int DataTable::ExecuteSqlRequest(const std::string& command, sqlite3_stmt **response)
{
    int ret_val = 0;
    std::shared_ptr<Database> l_data_base = m_data_base.lock();

    if(l_data_base == nullptr && l_data_base->m_sqlite_database == nullptr)
    {
        LOG(LOG_ERR,"ExecuteSqlRequest failed, database is nullptr\n");
    }
    else
    {
        if (SQLITE_OK != sqlite3_prepare_v2(l_data_base->m_sqlite_database, command.c_str(), -1, response, 0))
        {
            LOG(LOG_ERR,"SQL command error: %s\n", sqlite3_errmsg(l_data_base->m_sqlite_database));
            ret_val = -1;
        }
    }

    return ret_val;
}

int DataTable::NumberItems(int& number_items)
{
    int ret_val = -1;
    std::string command;
    std::shared_ptr<Database> l_data_base = m_data_base.lock();
    sqlite3_stmt *response;

    if(0 != FormNumberItemsMessage(command))
    {
        LOG(LOG_ERR,"Error forming NumberItems message\n");
    }
    else if(0 != ExecuteSqlRequest(command, &response))
    {
        LOG(LOG_ERR,"Failed to request number of items\n");
    }
    else if (0 != HandleNumberItemsResponse(&response, number_items))
    {
        LOG(LOG_ERR,"Failed to parse the response\n");
    }
    else
    {
        ret_val = 0;
    }

    return ret_val;
}

int DataTable::FormNumberItemsMessage(std::string& command)
{
    int ret_val = 0;

    /*
     * SELECT count(*) FROM tablename;
     */

    command = "SELECT count(*) FROM " + m_name ;

    return ret_val;
}

int DataTable::HandleNumberItemsResponse(sqlite3_stmt **response, int& number_items)
{
    int ret_val = 0;

    if(SQLITE_ROW != sqlite3_step(*response))
    {
        ret_val = -1;
    }
    else
    {
        number_items = sqlite3_column_int(*response, 0);
    }

    sqlite3_finalize(*response);

    return ret_val;
}

int DataTable::InsertItem(const Entry& item)
{
    int ret_val = 0;
    std::string command;

    if(0 != FormInsertItemMessage(command, item))
    {
        LOG(LOG_ERR,"Error forming InsertItem message\n");
    }
    else if(0 != ExecuteSqlCommand(command))
    {
        LOG(LOG_ERR,"Failed to delete table\n");
        ret_val = -1;
    }

    return ret_val;
}

int DataTable::FormInsertItemMessage(std::string& command, const Entry& item)
{
    int ret_val = 0;

    /*
     * INSERT INTO {table} ({var1.name}, {var2.name}) VALUES ({var1.value}, {var2.value})
     */

    command = "INSERT INTO " + m_name +  " (";

    for(auto it = item.cbegin(); it != item.cend(); std::advance(it,1))
    {
        command += it->name;
        if(it != std::prev(item.cend()))
        {
            command += ",";
        }
    }

    command += ") VALUES (";

    for(auto it = item.cbegin(); it != item.cend(); std::advance(it,1))
    {
        command += VariableToString(*it);

        if(it != std::prev(item.cend()))
        {
            command += ",";
        }

    }
    command += ");";

    return ret_val;
}

int DataTable::GetItem(Entry& item)
{
    int ret_val = -1;
    std::string command;
    std::shared_ptr<Database> l_data_base = m_data_base.lock();
    sqlite3_stmt *response;

    if(0 != FormGetItemMessage(command, item))
    {
        LOG(LOG_ERR,"Error forming GetItem message\n");
    }
    else if(0 != ExecuteSqlRequest(command, &response))
    {
        LOG(LOG_ERR,"Failed to request number of items\n");
    }
    else if (0 != HandleGetItemResponse(&response, item))
    {
        LOG(LOG_ERR,"Failed to parse the response\n");
    }
    else
    {
        ret_val = 0;
    }

    return ret_val;
}

int DataTable::FormGetItemMessage(std::string& command, const Entry& item)
{
    int ret_val = 0;

    /*
     * SELECT * FROM {table} WHERE id={id}
     */

    if(item.empty())
    {
        ret_val = -1;
    }
    else
    {
        command = "SELECT * FROM " + m_name +  " WHERE " + item.front().name + "=" + VariableToString(item.front());
    }

    return ret_val;
}

int DataTable::HandleGetItemResponse(sqlite3_stmt **response, Entry& item)
{
    int ret_val = 0;
    int i = 0;

    if(SQLITE_ROW != sqlite3_step(*response))
    {
        LOG(LOG_ERR,"Failed sqlite3_step\n");
        ret_val = -1;
    }
    else
    {
        for(auto it = item.begin(); it != item.end(); std::advance(it,1), i++)
        {
            std::string string_value = reinterpret_cast<const char *>(sqlite3_column_text(*response, i));
            StringToVariable(string_value, *it);
        }
    }

    sqlite3_finalize(*response);

    return ret_val;
}

std::string DataTable::VariableToString(const Variable& variable)
{
    std::string string_value;
    try
    {
        switch (variable.data_type)
        {
            case DataType::Integer:
                string_value = std::to_string(std::get<int>(variable.value));
                break;
            case DataType::Float:
                string_value = std::to_string(std::get<float>(variable.value));
                break;
            case DataType::String:
                string_value = "'";
                string_value += std::get<std::string>(variable.value);
                string_value += "'";
                break;
            case DataType::Boolean:
                string_value = "'";
                string_value += std::get<bool>(variable.value) == true ? "true" : "false";
                string_value += "'";
                break;
        }
    }
    catch(...)
    {
        LOG(LOG_ERR,"Failed to convert variable value to string value\n");
    }

    return string_value;
}

int DataTable::StringToVariable(const std::string& string_value, Variable& variable)
{
    int ret_val = 0;
    try
    {
        switch (variable.data_type)
        {
            case DataType::Integer:
                variable.value = std::stoi(string_value);
                break;
            case DataType::Float:
                variable.value = std::stof(string_value);
                break;
            case DataType::String:
                variable.value = string_value;
                break;
            case DataType::Boolean:
                variable.value = string_value == "true" ? true : false;
                break;
        }
    }
    catch(...)
    {
        LOG(LOG_ERR,"Failed to convert string value to variable value\n");
        ret_val = -1;
    }

    return ret_val;
}

int DataTable::SetItem(const Entry& item)
{
    int ret_val = 0;
    std::string command;

    if(0 != FormSetItemMessage(command, item))
    {
        LOG(LOG_ERR,"Error forming set Item message\n");
    }
    else if(0 != ExecuteSqlCommand(command))
    {
        LOG(LOG_ERR,"ExecuteSqlCommand failed\n");
        ret_val = -1;
    }

    return ret_val;
}

int DataTable::FormSetItemMessage(std::string& command, const Entry& item)
{
    int ret_val = 0;

    /*
     * UPDATE {tablename} SET {var2}={val2},{var3}={val3} WHERE {var1}={val1};
     */

    if(item.empty())
    {
        ret_val = -1;
    }
    else
    {
        command = "UPDATE " + m_name +  " SET ";

        for(auto it = std::next(item.cbegin()); it != item.cend(); std::advance(it,1))
        {
            command += it->name + "=" + VariableToString(*it);
            if(it != std::prev(item.cend()))
            {
                command += ",";
            }
        }

        command += " WHERE " + item.front().name + "=" + VariableToString(item.front());
    }

    return ret_val;
}

int DataTable::DeleteItem(const Entry& item)
{
    int ret_val = 0;
    std::string command;

    if(0 != FormDeleteItemMessage(command, item))
    {
        LOG(LOG_ERR,"Error forming set Item message\n");
    }
    else if(0 != ExecuteSqlCommand(command))
    {
        LOG(LOG_ERR,"ExecuteSqlCommand failed\n");
        ret_val = -1;
    }

    return ret_val;
}

int DataTable::FormDeleteItemMessage(std::string& command, const Entry& item)
{
    int ret_val = 0;

    /*
     * DELETE FROM {datatable} WHERE {var1}={val1};
     */

    command = "DELETE FROM " + m_name +  " WHERE " + item.front().name + "=" + VariableToString(item.front());

    return ret_val;
}

int DataTable::DeleteAllItems()
{
    int ret_val = 0;
    std::string command;

    if(0 != FormDeleteAllItemsMessage(command))
    {
        LOG(LOG_ERR,"Error forming set Item message\n");
    }
    else if(0 != ExecuteSqlCommand(command))
    {
        LOG(LOG_ERR,"ExecuteSqlCommand failed\n");
        ret_val = -1;
    }

    return ret_val;
}

int DataTable::FormDeleteAllItemsMessage(std::string& command)
{
    int ret_val = 0;

    /*
     * DELETE FROM {datatable};
     */

    command = "DELETE FROM " + m_name ;

    return ret_val;
}