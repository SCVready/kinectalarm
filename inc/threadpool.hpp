/**
 * @author Alejandro Solozabal
 *
 * @file threadpool.hpp
 *
 */

#ifndef THREADPOOL__H_
#define THREADPOOL__H_

/*******************************************************************
 * Includes
 *******************************************************************/
#include <iostream>
#include <thread>
#include <array>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <queue>

#include "log.hpp"

/*******************************************************************
 * Class declaration
 *******************************************************************/
class Task
{
    public:
        std::string m_name;
        std::mutex m_mutex;
        std::condition_variable m_condition_variable;
        bool m_ended = false;

    public:
        Task(std::string name) : m_name(name)
        {
        }

        virtual void operator() () = 0;

        void Join()
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            if(!m_ended)
            {
                LOG(LOG_DEBUG, "Join on Task: %s\n", m_name.c_str());
                m_condition_variable.wait(lock);
            }
            else
            {
                LOG(LOG_DEBUG, "Join on Task: %s, but it's ended\n", m_name.c_str());
            }
        }

        void ExecuteTask()
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            LOG(LOG_DEBUG, "Task: %s started\n", m_name.c_str());
            operator()();
            LOG(LOG_DEBUG, "Task: %s ended\n", m_name.c_str());
            m_ended = true;
            m_condition_variable.notify_one();
        }

        friend std::ostream& operator<<(std::ostream& os, const Task& task)
        {
            os << task.m_name;
            return os;
        }
};

template<int number_threads>
class ThreadPool
{
    private:
        std::array<std::unique_ptr<std::thread>, number_threads> m_threads;
        std::queue<std::shared_ptr<Task>> m_task_queue;
        std::mutex m_mutex;
        std::condition_variable m_condition_variable;
        bool m_running = true;

    public:
        ThreadPool()
        {
            LOG(LOG_INFO, "Threadpool created\n");
            std::unique_lock<std::mutex> lock(m_mutex);
            int thread_id = 0;
            for(auto& thread : m_threads)
            {
                thread = std::make_unique<std::thread>(&ThreadPool::ThreadLoop, this, thread_id);
                LOG(LOG_DEBUG, "Thread: %d created\n", thread_id++);
            }
        }

        ~ThreadPool()
        {
            {
                std::unique_lock<std::mutex> lock(m_mutex);
                LOG(LOG_INFO, "Threadpool ending\n");

                m_running = false;

                m_condition_variable.notify_all();
            }

            int thread_id = 0;
            for(auto& thread : m_threads)
            {
                thread->join();
                LOG(LOG_DEBUG, "Thread: %d destroyed\n", thread_id++);
            }
            LOG(LOG_INFO, "Threadpool destroyed\n");
        }

        int QueueTask(std::shared_ptr<Task> task)
        {
            int ret_val = 0;
            std::unique_lock<std::mutex> lock(m_mutex);
            if(!task->m_ended)
            {
                m_task_queue.push(task);
                LOG(LOG_DEBUG, "Added task: %s, Threadpool size: %u\n", task->m_name.c_str(), m_task_queue.size());
                m_condition_variable.notify_one();
            }
            else
            {
                LOG(LOG_ERR, "Error queueing an already ended task\n");
                ret_val = -1;
            }
            return ret_val;
        }

    private:
        void ThreadLoop(int thread_id)
        {
            while (m_running)
            {
                std::unique_lock<std::mutex> lock(m_mutex);
                if(!m_running)
                {
                    break;
                }

                LOG(LOG_DEBUG, "Thread: %d waiting\n", thread_id);
                if(!m_task_queue.empty())
                {
                    std::shared_ptr<Task> task = m_task_queue.front();
                    m_task_queue.pop();
                    LOG(LOG_DEBUG, "Threadpool size: %u\n", m_task_queue.size());
                    LOG(LOG_DEBUG, "Thread n: %d started task %s execution\n", thread_id, task->m_name.c_str());
                    lock.unlock();
                    task->ExecuteTask();
                    LOG(LOG_DEBUG, "Thread n: %d ended task %s execution\n", thread_id, task->m_name.c_str());
                }
                else
                {
                    m_condition_variable.wait(lock);
                    LOG(LOG_DEBUG, "Thread n: %d awake but with no task\n", thread_id);
                }

            }
        }
};

#endif /* THREADPOOL__H_ */
