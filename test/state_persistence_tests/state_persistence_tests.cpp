/**
 * @author Alejandro Solozabal
 *
 * @file liveview_tests.cpp
 *
 */

/*******************************************************************
 * Includes
 *******************************************************************/
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <memory>

#include "../../inc/state_persistence.hpp"

/*******************************************************************
 * Test class definition
 *******************************************************************/
using ::testing::_;
using ::testing::Return; 
using ::testing::NiceMock;
using ::testing::StrictMock;
using ::testing::SetArgReferee;
using ::testing::Ref;

class StatePersistenceTest : public ::testing::Test
{
public:
    std::shared_ptr<Database> database;

    StatePersistenceTest()
    {
        database = std::make_shared<Database>("test.db");
    }

    ~StatePersistenceTest()
    {
        database->RemoveDatabase();
    }

protected:
};

/*******************************************************************
 * Test cases
 *******************************************************************/
TEST_F(StatePersistenceTest, Contructor)
{
    ASSERT_NO_THROW(
        Database database1("test.db");
    );
}

TEST_F(StatePersistenceTest, CreateTable)
{
    ListOfVariables list_variables{
        {"Id",    DataType::Integer, 10},
        {"Name",  DataType::String,  "test"},
        {"Value", DataType::Float,   123}, /*TODO: Variant fails when using a float */
    };
    DataTable data_table(database, "testtable", list_variables);
}