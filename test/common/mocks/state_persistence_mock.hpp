#include <gmock/gmock.h>

#include "../../../inc/state_persistence_interface.hpp"

class DatabaseMock : public IDatabase
{
public:

    DatabaseMock() {};
    ~DatabaseMock() {};

    MOCK_METHOD(int, RemoveDatabase, ());

};

class DataTableMock : public IDataTable
{
public:

    DataTableMock() {};
    ~DataTableMock() {};

    MOCK_METHOD(int, NumberItems, (int& number_items));
    MOCK_METHOD(int, InsertItem, (const Entry& item));
    MOCK_METHOD(int, GetItem, (Entry& item));
    MOCK_METHOD(int, SetItem, (const Entry& item));
    MOCK_METHOD(int, DeleteItem, (const Entry& item));
    MOCK_METHOD(int, DeleteAllItems, ());
    MOCK_METHOD(int, DeleteTable, ());
};
