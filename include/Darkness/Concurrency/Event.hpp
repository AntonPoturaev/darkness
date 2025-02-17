/// Project          Darkness. C++ library.
/// Copyright (c)    2024 Poturaiev Anton. All rights reserved.
///
/// @file    Event.hpp
/// @authors Poturaiev Anton
/// @license Distributed under the Boost Software License, Version 1.0.
///		     See accompanying file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
/// @brief   Declaration of @class Event. More flexible realization of condition_variable...

#pragma once

#include <mutex>
#include <condition_variable>

namespace Darkness::Concurrency {
    template<
        typename StateT = bool
        , StateT _StateSignaledDefaultValue = true
        , StateT _StateNonSignaledValue = false
        , typename MutexT = std::mutex
        , typename WaitStrategyT = std::condition_variable
        , typename StatusForWaitStrategyT = std::cv_status
        , template<typename> class LockStrategyT = std::unique_lock
            >
    class Event
    {
    public:
        using tState = StateT;
        using tMonitor = MutexT;
        using tMonitorWaitStrategy = WaitStrategyT;
        using tMonitorLockStrategy = LockStrategyT<tMonitor>;

    public:
        Event() noexcept = default;

        explicit Event(tState initValue) noexcept
            : m_State(initValue)
        {
        }

        [[nodiscard]] tState GetState() const noexcept
        {
            return m_State;
        }

        [[nodiscard]] tState Wait(bool reset = false)
        {
            tMonitorLockStrategy lock(m_Monitor);

            while (StateNonSignaled == m_State)
            {
                m_MonitorWaitStrategy.wait(lock);
            }

            tState returnValue = m_State;

            if (reset)
            {
                m_State = StateNonSignaled;
            }

            return returnValue;
        }

        template<typename DurationT>
        [[nodiscard]] tState WaitFor(DurationT period, bool reset = false)
        {
            tMonitorLockStrategy lock(m_Monitor);

            while (StateNonSignaled == m_State)
            {
                if (StatusForWaitStrategyT::timeout == m_MonitorWaitStrategy.wait_for(lock, period))
                {
                    return m_State;
                }
            }

            tState returnValue = m_State;

            if (reset)
            {
                m_State = StateNonSignaled;
            }

            return returnValue;
        }

        void Set(tState setValue = StateSignaledDefault)
        {
            tMonitorLockStrategy const lock(m_Monitor);
            m_State = setValue;
            m_MonitorWaitStrategy.notify_all();
        }

        void Reset()
        {
            tMonitorLockStrategy const lock(m_Monitor);
            m_State = StateNonSignaled;
        }

    public:
        static constexpr tState const StateSignaledDefault = _StateSignaledDefaultValue;
        static constexpr tState const StateNonSignaled = _StateNonSignaledValue;

    private:
        std::atomic<tState> m_State { StateNonSignaled };
        tMonitor m_Monitor;
        tMonitorWaitStrategy m_MonitorWaitStrategy;
    };

    using tEvent = Event<>;
} /// namespace Darkness::Concurrency
