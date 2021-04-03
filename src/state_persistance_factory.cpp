/**
 * @author Alejandro Solozabal
 *
 * @file state_persistence_factory.cpp
 *
 */

/*******************************************************************
 * Includes
 *******************************************************************/
#include <memory>

#include "state_persistence_factory.hpp"
#include "state_persistence.hpp"

/*******************************************************************
 * Class definition
 *******************************************************************/

std::shared_ptr<IDatabase> StatePersistenceFactory::CreateDatabase(std::string path)
{
    return std::make_shared<Database>(path);
}

std::shared_ptr<IDataTable> StatePersistenceFactory::CreateDatatable(std::weak_ptr<Database> data_base, const std::string& name, Entry list_variables)
{
    return std::make_shared<DataTable>(data_base, name, list_variables);
}