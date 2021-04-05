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

#include "state_persistence.hpp"

/*******************************************************************
 * Class declaration
 *******************************************************************/
class StatePersistenceFactory
{
public:
    static std::shared_ptr<IDatabase> CreateDatabase(std::string path);
    static std::shared_ptr<IDataTable> CreateDatatable(std::weak_ptr<IDatabase> data_base,
                                                       const std::string& name,
                                                       const Entry list_variables);
};

#endif /* STATE_PERSISTANCE_FACTORY__H_ */