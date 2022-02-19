/**
 * @author Alejandro Solozabal
 *
 * @file cyclic_task_tests.cpp
 *
 */

/*******************************************************************
 * Includes
 *******************************************************************/
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <chrono>

#include "../../inc/cyclic_task.hpp"

/*******************************************************************
 * Test class definition
 *******************************************************************/
using ::testing::StrictMock;
using ::testing::AtMost;
using ::testing::Between;

class TaskMock
{
public:
    MOCK_METHOD(void, Task, ());
};

class CyclicTaskDumb : public CyclicTask
{
public:
    CyclicTaskDumb(std::string task_name, uint32_t loop_interval_ms) : CyclicTask(task_name, loop_interval_ms)
    {
    }

    void ExecutionCycle() override
    {
        task_mock.Task();
    }

    StrictMock<TaskMock> task_mock;
};

/*******************************************************************
 * Test cases
 *******************************************************************/
TEST(CyclicTaskTest, Contructor)
{
    ASSERT_NO_THROW(
        CyclicTaskDumb task("test", 100);
    );
}

TEST(CyclicTaskTest, StartStop)
{
    CyclicTaskDumb task("test", 100);

    EXPECT_CALL(task.task_mock, Task).Times(AtMost(1));

    EXPECT_EQ(0, task.Start());
    EXPECT_EQ(0, task.Stop());
}

TEST(CyclicTaskTest, IsRunning)
{
    CyclicTaskDumb task("test", 100);

    EXPECT_CALL(task.task_mock, Task).Times(AtMost(1));

    EXPECT_EQ(false, task.IsRunning());
    EXPECT_EQ(0, task.Start());
    EXPECT_EQ(true, task.IsRunning());
    EXPECT_EQ(0, task.Stop());
    EXPECT_EQ(false, task.IsRunning());
}

TEST(CyclicTaskTest, TaskExecution)
{
    CyclicTaskDumb task("test", 10);

    EXPECT_CALL(task.task_mock, Task).Times(AtMost(1));

    EXPECT_EQ(0, task.Start());

    EXPECT_CALL(task.task_mock, Task).Times(Between(10,11));
    std::this_thread::sleep_for (std::chrono::milliseconds(105));

    EXPECT_EQ(0, task.Stop());
}

TEST(CyclicTaskTest, CheckStoppingTimeLess1ms)
{
    int64_t max_stoping_time = 1;
    CyclicTaskDumb task("test", 100);

    EXPECT_CALL(task.task_mock, Task).Times(AtMost(1));

    EXPECT_EQ(0, task.Start());
    std::this_thread::sleep_for (std::chrono::milliseconds(50));
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    EXPECT_EQ(0, task.Stop());
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

    EXPECT_TRUE(max_stoping_time > std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count());
}
