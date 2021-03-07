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

std::shared_ptr<IStatePersistence> StatePersistenceFactory::Create()
{
    return std::make_shared<StatePersistence>();
}
