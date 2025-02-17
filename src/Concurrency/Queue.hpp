/// Project          Darkness. C++ library.
/// Copyright (c)    2024 Poturaiev Anton. All rights reserved.
///
/// @file    Queue.hpp
/// @authors Poturaiev Anton
/// @license Distributed under the Boost Software License, Version 1.0.
///		     See accompanying file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
/// @brief   Declaration of @class Queue which implements IQueue interface.

#pragma once

#include <Darkness/Concurrency/IQueue.hpp>

#include <deque>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

namespace Darkness::Concurrency {
    class Queue final : public IQueue
    {
        using tTaskQueue = std::deque<tTask>;
        using tUniquLock = std::unique_lock<std::mutex>;

    public:
        struct IExecutionPolicy
        {
            virtual ~IExecutionPolicy() = default;

            virtual void Start(Queue* queue) = 0;

            virtual bool RequestStop() = 0;

            [[nodiscard]] virtual std::stop_token GetStopToken() const noexcept = 0;
        };

        using tExecutionPolicyPtr = std::unique_ptr<IExecutionPolicy>;

        class MainThreadExecutionPolicy final : public IExecutionPolicy
        {
        public:
            void Start(Queue* queue) override;

            bool RequestStop() override;

            [[nodiscard]] std::stop_token GetStopToken() const noexcept override;

        private:
            std::stop_source m_StopSource { std::nostopstate };
        };

        class BackgroundThreadExecutionPolicy final : public IExecutionPolicy
        {
        public:
            void Start(Queue* queue) override;

            bool RequestStop() override;

            [[nodiscard]] std::stop_token GetStopToken() const noexcept override;

        private:
            std::jthread m_Worker {};
        };

    public:
        explicit Queue(std::string name, tExceptionHandler exceptionHandler
                       , tExecutionPolicyPtr executionPolicy) noexcept;

        ~Queue() override;

        Queue(Queue&&) = delete;

        Queue(Queue const&) = delete;

        Queue& operator=(Queue const&) = delete;

        Queue& operator=(Queue&&) = delete;

        void Start() override;

        void Stop() override;

        [[nodiscard]] eAsyncState GetState() const noexcept override;

        void Post(tTask&& task) override;

        void Post(tTask const& task) override;

        [[nodiscard]] std::thread::id GetWorkThreadId() const noexcept override;

        [[nodiscard]] std::string const& GetName() const noexcept override;

    private:
        void _Stop();

        void _DoStop();

        void _Routine(std::stop_token stopToken) noexcept;

    private:
        std::string const m_Name;
        tExceptionHandler const m_ExceptionHandler;
        tExecutionPolicyPtr m_ExecutionPolicy;
        std::atomic<eAsyncState> m_State;
        std::atomic<std::thread::id> m_Id;
        tTaskQueue m_TaskQueue;
        std::condition_variable m_Condition;
        std::mutex m_Mutex;
    };
} /// end namespace Darkness::Concurrency

