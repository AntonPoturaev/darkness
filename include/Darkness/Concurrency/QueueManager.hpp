/// Project          Darkness. C++ library.
/// Copyright (c)    2024 Poturaiev Anton. All rights reserved.
///
/// @file    QueueManager.hpp
/// @authors Poturaiev Anton
/// @license Distributed under the Boost Software License, Version 1.0.
///		     See accompanying file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
/// @brief   Declaration of @class QueueManager. Singleton for manipulations with IQueue objects.

#pragma once

#include <Darkness/Concurrency/IQueue.hpp>

namespace Darkness::Concurrency {
    class QueueManager final
    {
        class _Impl;

    public:
        ~QueueManager();

        [[nodiscard]] static QueueManager const& Instance() noexcept;

        [[nodiscard]] tQueueWeakPtr CreateOrGetBackgroundQueueByName(
            std::string const& name, tExceptionHandler const& exceptionHandler = {}) const;

        [[nodiscard]] bool IsExists(std::string const& name) const noexcept;

        [[nodiscard]] bool IsMainExists() const noexcept;

        [[nodiscard]] tQueueWeakPtr CreateOrGetMainQueue(tExceptionHandler const& exceptionHandler = {}) const;

        void ForgetByName(std::string const& name) const;

        void ForgetMainQueue() const;

        void KillAndForgetAll() const;

    private:
        QueueManager() noexcept;

    public:
        static std::string const mainQueueName;

    private:
        std::unique_ptr<_Impl> m_Impl;
    };
} /// end namespace Darkness::Concurrency
