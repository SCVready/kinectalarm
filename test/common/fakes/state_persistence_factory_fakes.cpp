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

#include "../../../inc/state_persistence_factory.hpp"

/*******************************************************************
 * Class definition
 *******************************************************************/
extern std::shared_ptr<IDataTable> g_data_table_mock;

std::shared_ptr<IDatabase> StatePersistenceFactory::CreateDatabase(std::string path)
{
    return nullptr;
}

std::shared_ptr<IDataTable> StatePersistenceFactory::CreateDatatable(std::weak_ptr<IDatabase> data_base, const std::string& name, const Entry list_variables)
{
    return g_data_table_mock;
}
