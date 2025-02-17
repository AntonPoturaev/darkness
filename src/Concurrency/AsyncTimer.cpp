/// Project          Darkness. C++ library.
/// Copyright (c)    2024 Poturaiev Anton. All rights reserved.
///
/// @file    AsyncTimer.cpp
/// @authors Poturaiev Anton
/// @license Distributed under the Boost Software License, Version 1.0.
///		     See accompanying file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
/// @brief   Implementation of @class AsyncTimer

#include <Darkness/Concurrency/AsyncTimer.hpp>
#include <Darkness/Concurrency/Utilities.hpp>
#include <Darkness/Common/Utilities.hpp>

#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <cassert>
#include <iostream>
#include <iomanip>
#include <utility>
#include <variant>

namespace Darkness::Concurrency {
    class AsyncTimer::_Impl final
    {
        using tUniquLock = std::unique_lock<std::mutex>;

        struct _Params final
        {
            using tDelayProviderHolder = std::variant<tDurationDelayRuntimeProvider, tDurationDelay>;

        public:
            explicit _Params(tDelayProviderHolder delayProvider, tTask task
                             , std::string name, tExceptionHandler exceptionHandler) noexcept
                : delayProviderHolder { std::move(delayProvider) }
                  , task { std::move(task) }
                  , exceptionHandler { std::move(exceptionHandler) }
                  , name { std::move(name) }
            {
            }

        public:
            tDelayProviderHolder delayProviderHolder;
            tTask task;
            tExceptionHandler exceptionHandler;
            std::string name;
        };

        using tParamsOpt = std::optional<_Params>;

        struct _ExecutionContext final
        {
        public:
            explicit _ExecutionContext(_Impl* self) noexcept
                : condition {}
                  , mutex {}
                  , thread([self](auto stopToken) { self->_Routine(stopToken); })
            {
            }

            void _Stop() noexcept
            {
#if defined(Darkness_Concurrency_Timer_DEBUG)
                /// @short Other way to check for a call itself from this thread.
                auto const callerThreadId = std::this_thread::get_id();
                assert(callerThreadId != id && "Bad logic! The thread is stopped from another thread!");
                if (callerThreadId == id)
                {
                    std::cerr << "Bad logic! The thread is stopped from another thread!" << '\n';
                }
#endif /// Darkness_Concurrency_Timer_DEBUG
                thread.request_stop();
                condition.notify_all();
            }

        public:
            std::condition_variable condition;
            std::mutex mutex;
            std::jthread thread;
            std::atomic<std::thread::id> id {};
        };

        using tExecutionContextPtr = std::unique_ptr<_ExecutionContext>;

    public:
        _Impl() noexcept = default;

        explicit _Impl(tDurationDelayRuntimeProvider const& durationDelayRuntimeProvider, tTask task
                       , std::string name, tExceptionHandler exceptionHandler) noexcept
            : m_Params(std::make_optional<_Params>(durationDelayRuntimeProvider, std::move(task)
                                                   , std::move(name), std::move(exceptionHandler)))
        {
        }

        explicit _Impl(tDurationDelay const& durationDelay, tTask task
                       , std::string name, tExceptionHandler exceptionHandler) noexcept
            : m_Params(std::make_optional<_Params>(durationDelay, std::move(task)
                                                   , std::move(name), std::move(exceptionHandler)))
        {
        }

        ~_Impl() noexcept
        {
            _Stop();
        }

        _Impl(_Impl const&) = delete;

        _Impl(_Impl&& other) noexcept
            : m_Params { std::move(other.m_Params) }
              , m_ExecutionContext { std::move(other.m_ExecutionContext) }
              , m_State { other.m_State.load() }
        {
            other.m_State = eAsyncState::Free;
        }

        _Impl& operator=(_Impl const&) = delete;

        _Impl& operator=(_Impl&& other) noexcept
        {
            if (this != &other)
            {
                m_Params = std::move(other.m_Params);
                m_ExecutionContext = std::move(other.m_ExecutionContext);
                m_State = other.m_State.load();
                other.m_State = eAsyncState::Free;
            }

            return *this;
        }

        [[nodiscard]] eAsyncState _GetState() const noexcept
        {
            return m_State;
        }

        void _Start() noexcept
        {
            switch (m_State)
            {

                case eAsyncState::Free:
                case eAsyncState::Stopped:
                {
                    m_ExecutionContext = std::make_unique<_ExecutionContext>(this);
                    break;
                }

                case eAsyncState::Busy:
#if defined(Darkness_Concurrency_Timer_DEBUG)
                    std::cerr << "AsyncTimer.Start is unavailable. The timer " << std::quoted(m_Params->name)
                              << " is already started!" << '\n';
#endif /// Darkness_Concurrency_Timer_DEBUG
                    break;

                case eAsyncState::Stopping:
#if defined(Darkness_Concurrency_Timer_DEBUG)
                    std::cerr << "AsyncTimer.Start is unavailable. The timer " << std::quoted(m_Params->name)
                              << " is in the process stopping..." << '\n';
#endif /// Darkness_Concurrency_Timer_DEBUG
                    break;
            }
        }

        void _Stop() noexcept
        {
            switch (m_State)
            {
                case eAsyncState::Free:
                case eAsyncState::Stopped:
                {
#if defined(Darkness_Concurrency_Timer_DEBUG)
                    std::cerr << "AsyncTimer.Stop has no effect. The timer " << std::quoted(m_Params->name)
                              << " is already stopped." << '\n';
#endif /// Darkness_Concurrency_Timer_DEBUG
                    break;
                }

                case eAsyncState::Busy:
                {
                    m_State = eAsyncState::Stopping;
                    m_ExecutionContext->_Stop();
                    m_ExecutionContext.reset();
                    break;
                }

                case eAsyncState::Stopping:
                {
#if defined(Darkness_Concurrency_Timer_DEBUG)
                    std::cerr << "AsyncTimer.Stop is unavailable. The timer " << std::quoted(m_Params->name)
                              << " is in the process stopping..." << '\n';
#endif /// Darkness_Concurrency_Timer_DEBUG
                    break;
                }
            }
        }

    private:
        void _Routine(std::stop_token stopToken) noexcept
        {
            assert(m_Params && "Bad data!");
            assert(m_ExecutionContext && "Bad data!");

            if (!m_Params->name.empty())
            {
                SetCurrentThreadName(m_Params->name);
            }

            m_ExecutionContext->id = std::this_thread::get_id();
            m_State = eAsyncState::Busy;

            Common::ScopeExit const scopeExit { [this] {
                m_State = eAsyncState::Stopped;
            }};

            std::exception_ptr exceptionPtr {};

            try
            {
                tUniquLock lock(m_ExecutionContext->mutex);
                bool isStopped = false; /// For checking the false wake up...
                do
                {
                    auto const delay = std::visit([this](auto&& delayProviderHolder) {
                        using tCurrentDelayProvider = std::decay_t<decltype(delayProviderHolder)>;
                        if constexpr (std::is_same_v<tCurrentDelayProvider, tDurationDelayRuntimeProvider>)
                        {
                            return delayProviderHolder();
                        }
                        else
                        {
                            return delayProviderHolder;
                        }
                    }, m_Params->delayProviderHolder);

                    isStopped = m_ExecutionContext->condition.wait_for(
                        lock, delay, [&stopToken] {
                            return stopToken.stop_requested();
                        });

                    if (!isStopped || !stopToken.stop_requested())
                    {
                        if (m_Params->task)
                        {
                            m_Params->task();
                        }
                    }

                    if (stopToken.stop_requested())
                    {
                        isStopped = true;
                    }
                }
                while (!isStopped || !stopToken.stop_requested());
            }
            catch (...)
            {
                exceptionPtr = std::current_exception();

#if defined(Darkness_Concurrency_Timer_DEBUG)
                DebugExceptionHandler(exceptionPtr);
#endif /// Darkness_Concurrency_Timer_DEBUG

                /// @short We can select any logic...
                /// @{
#if 1
                /// 1) Call an exception handler and continue the thread.
                if (exceptionPtr && m_Params->exceptionHandler)
                {
                    m_Params->exceptionHandler(exceptionPtr);
                }
#else
                /// 2) Stop the thread and call the exception handler.
                m_ExecutionContext->thread.request_stop();
                m_IsStopped = true;

                if (exceptionPtr && m_Params->exceptionHandler)
                {
                    m_Params->exceptionHandler(exceptionPtr);
                }
#endif
                /// @}
            }
        }

    private:
        tParamsOpt m_Params { std::nullopt };
        tExecutionContextPtr m_ExecutionContext {};
        std::atomic<eAsyncState> m_State { eAsyncState::Free };
    };

    AsyncTimer::AsyncTimer(tDurationDelayRuntimeProvider const& durationDelayRuntimeProvider, tTask task
                           , std::string name, tExceptionHandler exceptionHandler) noexcept(false)
        : m_Impl(std::make_unique<_Impl>(durationDelayRuntimeProvider, std::move(task)
                                         , std::move(name), std::move(exceptionHandler)))
    {
        assert(durationDelayRuntimeProvider && "Bad data!");
        if (!durationDelayRuntimeProvider)
        {
            throw std::invalid_argument(
                "Darkness::Concurrency::AsyncTimer::_Impl::ctor: durationDelayRuntimeProvider is invalid!");
        }
    }

    AsyncTimer::AsyncTimer() noexcept
        : m_Impl(std::make_unique<_Impl>())
    {
    }

    AsyncTimer::~AsyncTimer() noexcept = default;

    AsyncTimer::AsyncTimer(AsyncTimer&& other) noexcept
        : m_Impl(std::move(other.m_Impl))
    {
    }

    AsyncTimer& AsyncTimer::operator=(AsyncTimer&& other) noexcept
    {
        if (this != &other)
        {
            m_Impl = std::move(other.m_Impl);
        }

        return *this;
    }

    eAsyncState AsyncTimer::GetState() const noexcept
    {
        return m_Impl->_GetState();
    }

    void AsyncTimer::Start() noexcept
    {
        m_Impl->_Start();
    }

    void AsyncTimer::Stop() noexcept
    {
        m_Impl->_Stop();
    }

    AsyncTimer::AsyncTimer(tDurationDelay const& durationDelay, tTask task
                           , std::string name, tExceptionHandler exceptionHandler) noexcept(false)
        : m_Impl(std::make_unique<_Impl>(durationDelay, std::move(task), std::move(name), std::move(exceptionHandler)))
    {
    }
} /// namespace Darkness::Concurrency
