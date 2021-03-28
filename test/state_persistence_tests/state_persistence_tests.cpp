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
    std::shared_ptr<Database> m_database;

    const Entry m_table1_item_def{
        {"Var0", DataType::Integer,},
        {"Var1", DataType::String,},
        {"Var2", DataType::Float,},
    };

    const Entry m_table1_item_1{
        {"Var0", DataType::Integer, 10},
        {"Var1", DataType::String,  "test"},
        {"Var2", DataType::Float,   123.23f},
    };

    const Entry m_table1_item_2{
        {"Var0", DataType::Integer, 20},
        {"Var1", DataType::String,  "test2"},
        {"Var2", DataType::Float,   288.28f},
    };

    StatePersistenceTest()
    {
        m_database = std::make_shared<Database>("test.db");
    }

    ~StatePersistenceTest()
    {
        m_database->RemoveDatabase();
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
    ASSERT_NO_THROW(
        DataTable data_table(m_database, "testtable", m_table1_item_def);
    );
}


TEST_F(StatePersistenceTest, DeleteTable)
{
    DataTable data_table(m_database, "testtable", m_table1_item_def);
    EXPECT_EQ(0, data_table.DeleteTable());
}

TEST_F(StatePersistenceTest, InsertItems)
{
    DataTable data_table(m_database, "testtable", m_table1_item_def);
    EXPECT_EQ(0, data_table.InsertItem(m_table1_item_1));
}

TEST_F(StatePersistenceTest, NumberItems)
{
    int number_items = 0;

    DataTable data_table(m_database, "testtable", m_table1_item_def);

    EXPECT_EQ(0, data_table.NumberItems(number_items));
    EXPECT_EQ(0, number_items);

    EXPECT_EQ(0, data_table.InsertItem(m_table1_item_1));

    EXPECT_EQ(0, data_table.NumberItems(number_items));
    EXPECT_EQ(1, number_items);

    EXPECT_EQ(0, data_table.InsertItem(m_table1_item_2));

    EXPECT_EQ(0, data_table.NumberItems(number_items));
    EXPECT_EQ(2, number_items);
}

TEST_F(StatePersistenceTest, GetItems)
{
    DataTable data_table(m_database, "testtable", m_table1_item_def);
    EXPECT_EQ(0, data_table.InsertItem(m_table1_item_1));

    Entry item1{
        {"Var0", DataType::Integer,10},
        {"Var1", DataType::String,},
        {"Var2", DataType::Float,},
    };
    EXPECT_EQ(0, data_table.GetItem(item1));

    float val = std::get<float>(m_table1_item_1.back().value);
    float val_read = std::get<float>(item1.back().value);

    EXPECT_EQ(val, val_read);
}