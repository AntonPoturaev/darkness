/// Project          Darkness. C++ library.
/// Copyright (c)    2024 Poturaiev Anton. All rights reserved.
///
/// @file    AsyncTimer.hpp
/// @authors Poturaiev Anton
/// @license Distributed under the Boost Software License, Version 1.0.
///		     See accompanying file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
/// @brief   Declaration of @class AsyncTimer. Provides the ability to execute a task at a specified interval.

#pragma once

#include <Darkness/Concurrency/Types.hpp>

#include <chrono>
#include <string>
#include <memory>

namespace Darkness::Concurrency {
    class AsyncTimer final
    {
        class _Impl;

    public:
        using tDurationDelay = std::chrono::nanoseconds;
        using tDurationDelayRuntimeProvider = std::function<tDurationDelay()>;

    public:
        /// @brief Constructs an AsyncTimer object.
        /// @param durationDelay The constant time interval at which the task will be executed.
        /// @param task The task to be executed. Maybe empty.
        /// @param name The name of the timer thread into the OS. Maybe empty.
        /// @param exceptionHandler The exception handler for the task. Maybe empty.
        ///        @short If an exception will be occurred then a timer object will store result of call
        ///               std::current_exception(), after that it will be stopped and stored exception will
        ///               passed to an exceptionHandler.
        template<typename Representation, typename Period>
        explicit AsyncTimer(std::chrono::duration<Representation, Period> const& durationDelay, tTask task
                            , std::string name = "", tExceptionHandler exceptionHandler = {}) noexcept(false)
            : AsyncTimer(std::chrono::duration_cast<tDurationDelay>(durationDelay)
                         , std::move(task), std::move(name), std::move(exceptionHandler))
        {
        }

        /// @brief Constructs an AsyncTimer object.
        /// @param durationDelayRuntimeProvider The callback which will be provide a time interval at by user logic.
        ///                                     Must be valid. Otherwise, std::invalid_argument will be thrown.
        /// @param task The task to be executed. Maybe empty.
        /// @param name The name of the timer thread into the OS. Maybe empty.
        /// @param exceptionHandler The exception handler for the task. Maybe empty.
        ///        @short If an exception will be occurred then a timer object will store result of call
        ///               std::current_exception(), after that it will be stopped and stored exception will
        ///               passed to an exceptionHandler.
        explicit AsyncTimer(tDurationDelayRuntimeProvider const& durationDelayRuntimeProvider, tTask task
                            , std::string name = "", tExceptionHandler exceptionHandler = {}) noexcept(false);

        AsyncTimer() noexcept;

        /// @short Will be stopped before destruction.
        ~AsyncTimer() noexcept;

        AsyncTimer(AsyncTimer const&) = delete;

        AsyncTimer(AsyncTimer&& other) noexcept;

        AsyncTimer& operator=(AsyncTimer const&) = delete;

        AsyncTimer& operator=(AsyncTimer&& other) noexcept;

        /// @brief Returns the status of the timer.
        [[nodiscard]] eAsyncState GetState() const noexcept;

        /// @brief Starts the timer.
        void Start() noexcept;

        /// @brief Stops the timer.
        /// @warning Should not be called from the timer task.
        void Stop() noexcept;

    private:
        explicit AsyncTimer(tDurationDelay const& durationDelay, tTask task
                            , std::string name = "", tExceptionHandler exceptionHandler = {}) noexcept(false);

    private:
        std::unique_ptr<_Impl> m_Impl;
    };
} /// namespace Darkness::Concurrency
