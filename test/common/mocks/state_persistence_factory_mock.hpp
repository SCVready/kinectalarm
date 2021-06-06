#include <gmock/gmock.h>

#include "state_persistence_factory.hpp"
#include "state_persistence.hpp"

class StatePersistenceFactoryMock
{
public:

    StatePersistenceFactoryMock();
    virtual ~StatePersistenceFactoryMock();

    MOCK_METHOD(std::shared_ptr<IDataTable>, CreateDatatable, (std::weak_ptr<IDatabase> data_base, const std::string& name, const Entry list_variables));
};