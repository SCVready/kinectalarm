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
#include "../mocks/state_persistence_factory_mock.hpp"

/*******************************************************************
 * Class definition
 *******************************************************************/
extern std::shared_ptr<StatePersistenceFactoryMock> g_state_persistence_factory_mock;

std::shared_ptr<IDatabase> StatePersistenceFactory::CreateDatabase(std::string path)
{
    return nullptr;
}

std::shared_ptr<IDataTable> StatePersistenceFactory::CreateDatatable(std::weak_ptr<IDatabase> data_base, const std::string& name, const Entry list_variables)
{
    return g_state_persistence_factory_mock->CreateDatatable(data_base, name, list_variables);
}
