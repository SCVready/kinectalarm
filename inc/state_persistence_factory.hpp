/**
 * @author Alejandro Solozabal
 *
 * @file state_persistence_factory.hpp
 *
 */

#ifndef STATE_PERSISTANCE_FACTORY__H_
#define STATE_PERSISTANCE_FACTORY__H_

/*******************************************************************
 * Includes
 *******************************************************************/
#include <memory>

#include "state_persistence_interface.hpp"

/*******************************************************************
 * Class declaration
 *******************************************************************/
class StatePersistenceFactory
{
public:
    static std::shared_ptr<IStatePersistence> Create();
};

#endif /* STATE_PERSISTANCE_FACTORY__H_ */