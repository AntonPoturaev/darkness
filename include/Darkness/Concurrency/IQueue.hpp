/// Project          Darkness. C++ library.
/// Copyright (c)    2024 Poturaiev Anton. All rights reserved.
///
/// @file    IQueue.hpp
/// @authors Poturaiev Anton
/// @license Distributed under the Boost Software License, Version 1.0.
///		     See accompanying file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
/// @brief   Declaration of @abstract @interface IQueue which present simple and usable interface for work a task queue.

#pragma once

#include <Darkness/Concurrency/Types.hpp>

#include <thread>
#include <memory>
#include <stop_token>

namespace Darkness::Concurrency {
    struct IQueue
    {
        virtual ~IQueue() = default;

        virtual void Start() = 0;

        virtual void Stop() = 0;

        [[nodiscard]] virtual eAsyncState GetState() const noexcept = 0;

        virtual void Post(tTask&& task) = 0;

        virtual void Post(tTask const& task) = 0;

        [[nodiscard]] virtual std::thread::id GetWorkThreadId() const noexcept = 0;

        [[nodiscard]] virtual std::string const& GetName() const noexcept = 0;
    };

    using tQueuePtr = std::shared_ptr<IQueue>;
    using tQueueWeakPtr = std::weak_ptr<IQueue>;
} /// end namespace Darkness::Concurrency
