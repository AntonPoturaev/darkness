/// Project          Darkness. C++ library.
/// Copyright (c)    2024 Poturaiev Anton. All rights reserved.
///
/// @file    PriorityQueueScheduler.hpp
/// @authors Poturaiev Anton
/// @license Distributed under the Boost Software License, Version 1.0.
///		     See accompanying file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
/// @brief   Implementation of @class PriorityQueueScheduler.

#pragma once

#include <concepts>
#include <coroutine>
#include <functional>
#include <queue>
#include <utility>

namespace Darkness::Concurrency::Coroutine {
    class Task final
    {
    public:
        struct promise_type final
        {
            constexpr std::suspend_always initial_suspend() noexcept
            {
                return {};
            }

            constexpr std::suspend_always final_suspend() noexcept
            {
                return {};
            }

            Task get_return_object()
            {
                return std::coroutine_handle<promise_type>::from_promise(*this);
            }

            constexpr void return_void()
            {}

            constexpr void unhandled_exception()
            {}
        };

    public:
        constexpr Task(std::coroutine_handle<promise_type> handle) noexcept
            : m_Handle { handle }
        {
        }

        std::coroutine_handle<promise_type> get_handle() noexcept
        {
            return m_Handle;
        }

    private:
        std::coroutine_handle<promise_type> m_Handle;
    };

    template<typename PriorityT = int>
        requires std::integral<PriorityT>
    using Job = std::pair<PriorityT, std::coroutine_handle<>>;

    template<
        typename PriorityT = int
        , typename UpdaterT = std::identity
        , typename ComparatorT = std::ranges::less>
    requires
    std::invocable<decltype(UpdaterT()), PriorityT>
    && std::predicate<decltype(ComparatorT()), Job<PriorityT>, Job<PriorityT>>
    class PriorityQueueScheduler final
    {
    public:
        using tPriority = PriorityT;

    private:
        using tJob = Job<tPriority>;
        using tJobQueueContainer = std::vector<tJob>;
        using tJobQueue = std::priority_queue<tJob, tJobQueueContainer, ComparatorT>;

    public:
        void AddTask(tPriority priority, std::coroutine_handle<> task)
        {
            m_JobQueue.emplace(priority, task);
        }

        void Schedule()
        {
            UpdaterT updater = {};
            while (!m_JobQueue.empty())
            {
                auto [priority, task] = m_JobQueue.top();
                m_JobQueue.pop();
                task.resume();

                if (task.done())
                {
                    task.destroy();
                }
                else
                {
                    m_JobQueue.emplace(updater(priority), task);
                }
            }
        }

    private:
        tJobQueue m_JobQueue;
    };
} /// end namespace Darkness::Concurrency::Coroutine

/**
 * @code
    Task createTask(const std::string& name)
    {
        std::cout << name << " start\n";

        co_await std::suspend_always();

        for (size_t i = 0; i <= 3; ++i)
        {
            std::cout << name << " execute " << i << "\n";

            co_await std::suspend_always();
        }

        co_await std::suspend_always();

        std::cout << name << " finish\n";
    }

    void CoroutinePriorityQueueScheduler()
    {
        std::cout << '\n';

        std::string const taskA = "TaskA";     // (1)
        std::string const taskB = "  TaskB";   // (2)
        std::string const taskC = "    TaskC"; // (3)

        PriorityQueueScheduler<> scheduler1;

        scheduler1.AddTask(0, createTask(taskA).get_handle());
        scheduler1.AddTask(1, createTask(taskB).get_handle());
        scheduler1.AddTask(2, createTask(taskC).get_handle());

        scheduler1.Schedule();

        std::cout << '\n';

        PriorityQueueScheduler<int, decltype([](int a) { return a - 1; })> scheduler2;

        scheduler2.AddTask(0, createTask(taskA).get_handle());
        scheduler2.AddTask(1, createTask(taskB).get_handle());
        scheduler2.AddTask(2, createTask(taskC).get_handle());

        scheduler2.Schedule();

        std::cout << std::endl;
    }
*/
