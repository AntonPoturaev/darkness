/// Project          Darkness. C++ library.
/// Copyright (c)    2024 Poturaiev Anton. All rights reserved.
///
/// @file    Queue.cpp
/// @authors Poturaiev Anton
/// @license Distributed under the Boost Software License, Version 1.0.
///		     See accompanying file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
/// @brief   Implementation of @class Queue

#include "Queue.hpp"
#include <Darkness/Common/Utilities.hpp>
#include <Darkness/Concurrency/Utilities.hpp>
#include <Darkness/Concurrency/QueueManager.hpp>

#include <cassert>
#include <iostream>
#include <iomanip>

namespace Darkness::Concurrency {

    void Queue::MainThreadExecutionPolicy::Start(Queue* queue)
    {
        assert(queue && "Bad data!");
        if (queue)
        {
            m_StopSource = {};
            queue->_Routine(m_StopSource.get_token());
        }
    }

    bool Queue::MainThreadExecutionPolicy::RequestStop()
    {
        return m_StopSource.request_stop();
    }

    std::stop_token Queue::MainThreadExecutionPolicy::GetStopToken() const noexcept
    {
        return m_StopSource.get_token();
    }

    void Queue::BackgroundThreadExecutionPolicy::Start(Queue* queue)
    {
        assert(queue && "Bad data!");
        if (queue)
        {
            m_Worker = std::jthread([queue](std::stop_token stopToken) {
                queue->_Routine(std::move(stopToken));
            });
        }
    }

    bool Queue::BackgroundThreadExecutionPolicy::RequestStop()
    {
        return m_Worker.request_stop();
    }

    std::stop_token Queue::BackgroundThreadExecutionPolicy::GetStopToken() const noexcept
    {
        return m_Worker.get_stop_token();
    }

    Queue::Queue(std::string name, tExceptionHandler exceptionHandler
                 , tExecutionPolicyPtr executionPolicy) noexcept
        : m_Name(std::move(name))
          , m_ExceptionHandler(std::move(exceptionHandler))
          , m_ExecutionPolicy(std::move(executionPolicy))
          , m_State(eAsyncState::Free)
    {
        assert(!m_Name.empty() && "Bad data!");
        assert(m_ExecutionPolicy && "Bad data!");
    }

    Queue::~Queue()
    {
        _Stop();
    }

    void Queue::Start()
    {
        switch (m_State)
        {
            case eAsyncState::Free:
            case eAsyncState::Stopped:
            {
                m_ExecutionPolicy->Start(this);
                break;
            }

            case eAsyncState::Busy:
#if defined(Darkness_Concurrency_Queue_DEBUG)
                std::cerr << "Queue.Start is unavailable. The queue " << std::quoted(m_Name)
                          << " is already started!" << '\n';
#endif /// Darkness_Concurrency_Queue_DEBUG
                break;

            case eAsyncState::Stopping:
#if defined(Darkness_Concurrency_Queue_DEBUG)
                std::cerr << "Queue.Start is unavailable. The queue " << std::quoted(m_Name)
                          << " is in the process stopping..." << '\n';
#endif /// Darkness_Concurrency_Queue_DEBUG
                break;
        }
    }

    void Queue::Stop()
    {
        _Stop();
    }

    eAsyncState Queue::GetState() const noexcept
    {
        return m_State;
    }

    void Queue::Post(tTask&& task)
    {
        tUniquLock const lock(m_Mutex);
        m_TaskQueue.push_back(std::forward<tTask>(task));
        m_Condition.notify_one();
    }

    void Queue::Post(tTask const& task)
    {
        tUniquLock const lock(m_Mutex);
        m_TaskQueue.push_back(task);
        m_Condition.notify_one();
    }

    std::thread::id Queue::GetWorkThreadId() const noexcept
    {
        return m_Id;
    }

    std::string const& Queue::GetName() const noexcept
    {
        return m_Name;
    }

    void Queue::_Stop()
    {
        switch (m_State)
        {
            case eAsyncState::Free:
            case eAsyncState::Stopped:
            {
#if defined(Darkness_Concurrency_Queue_DEBUG)
                std::cerr << "Queue.Stop has no effect. The queue " << std::quoted(m_Name)
                          << " is already stopped." << '\n';
#endif /// Darkness_Concurrency_Queue_DEBUG
                break;
            }

            case eAsyncState::Busy:
            {
                _DoStop();
                break;
            }

            case eAsyncState::Stopping:
            {
#if defined(Darkness_Concurrency_Queue_DEBUG)
                std::cerr << "Queue.Stop is unavailable. The queue " << std::quoted(m_Name)
                          << " is in the process stopping..." << '\n';
#endif /// Darkness_Concurrency_Queue_DEBUG
                break;
            }
        }
    }

    void Queue::_DoStop()
    {
        tUniquLock const lock(m_Mutex);

#if defined(Darkness_Concurrency_Queue_DEBUG)
        /// @short Other way to check for a call itself from this thread.
        auto const callerThreadId = std::this_thread::get_id();
        assert(callerThreadId != m_Id && "Bad logic! The thread is stopped from another thread!");
        if (callerThreadId == m_Id)
        {
            std::cerr << "Bad logic! The thread is stopped from another thread!" << '\n';
        }
#endif /// Darkness_Concurrency_Queue_DEBUG

        m_State = eAsyncState::Stopping;

        m_TaskQueue.clear();

        bool const stopPossible = m_ExecutionPolicy->GetStopToken().stop_possible();
        assert(stopPossible && "Bad logic!");

        bool const stopRequested = m_ExecutionPolicy->RequestStop();
        assert(stopRequested && "Bad logic!");

        m_Condition.notify_all();
    }

    void Queue::_Routine(std::stop_token stopToken) noexcept
    {
        m_Id = std::this_thread::get_id();
        m_State = eAsyncState::Busy;
        Common::ScopeExit const scopeExit { [this] {
            m_Id.store({});
            m_State = eAsyncState::Stopped;
        }};

        assert(!m_Name.empty() && "Bad data!");
        if (!m_Name.empty()
#if 0
            && m_Name != QueueManager::mainQueueName
#endif
            )
        {
            SetCurrentThreadName(m_Name);
        }

        std::exception_ptr exceptionPtr {};

        try
        {
            while (!stopToken.stop_requested())
            {
                tTask task;
                {
                    tUniquLock lock(m_Mutex);
                    m_Condition.wait(lock, [this, &task, &stopToken] {
                        if (stopToken.stop_requested())
                        {
                            return true;
                        }

                        if (!m_TaskQueue.empty())
                        {
                            task = std::move(m_TaskQueue.front());
                            m_TaskQueue.pop_front();
                            return true;
                        }

                        return false; /// Continue waiting...
                    });
                }

                if (task)
                {
                    try
                    {
                        task();
                    }
                    catch (...)
                    {
                        exceptionPtr = std::current_exception();

                        if (m_ExceptionHandler)
                        {
                            m_ExceptionHandler(exceptionPtr);
                        }
                    }
                }
            }
        }
        catch (...)
        {
            exceptionPtr = std::current_exception();

            if (m_ExceptionHandler)
            {
                m_ExceptionHandler(exceptionPtr);
            }
        }
    }

} /// end namespace Darkness::Concurrency
