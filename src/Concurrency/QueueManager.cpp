/// Project          Darkness. C++ library.
/// Copyright (c)    2024 Poturaiev Anton. All rights reserved.
///
/// @file    QueueManager.cpp
/// @authors Poturaiev Anton
/// @license Distributed under the Boost Software License, Version 1.0.
///		     See accompanying file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
/// @brief   Implementation of @class QueueManager

#include <Darkness/Concurrency/QueueManager.hpp>
#include "Queue.hpp"

#include <unordered_map>
#include <mutex>

namespace Darkness::Concurrency {
    class QueueManager::_Impl final
    {
        using tQueueStore = std::unordered_map<std::string, tQueuePtr>;

        using tAccess = std::mutex;
        using tLock = std::lock_guard<tAccess> const;

    public:
        [[nodiscard]] bool _IsExists(std::string const& name) const noexcept
        {
            tLock lock(m_Access);
            return m_QueuesStore.contains(name);
        }

        [[nodiscard]] tQueueWeakPtr _CreateOrGetBackgroundQueueByName(
            std::string const& name, tExceptionHandler const& exceptionHandler)
        {
            tLock lock(m_Access);

            auto found = m_QueuesStore.find(name);
            if (found == m_QueuesStore.end())
            {
                Queue::tExecutionPolicyPtr executionPolicy;
                if (name == QueueManager::mainQueueName)
                {
                    executionPolicy = std::make_unique<Queue::MainThreadExecutionPolicy>();
                }
                else
                {
                    executionPolicy = std::make_unique<Queue::BackgroundThreadExecutionPolicy>();
                }

                return m_QueuesStore[name] = std::make_shared<Queue>(
                    name, exceptionHandler, std::move(executionPolicy));
            }

            return found->second;
        }

        void _ForgetByName(std::string const& name)
        {
            tLock lock(m_Access);
            m_QueuesStore.erase(name);
        }

        void _KillAndForgetAll()
        {
            tLock lock(m_Access);
            m_QueuesStore.clear();
        }

    private:
        tQueueStore m_QueuesStore;
        tAccess mutable m_Access;
    };

    QueueManager::~QueueManager() = default;

    QueueManager const& QueueManager::Instance() noexcept
    {
        static QueueManager instance;
        return instance;
    }

    tQueueWeakPtr QueueManager::CreateOrGetBackgroundQueueByName(std::string const& name
                                                                 , tExceptionHandler const& exceptionHandler) const
    {
        return m_Impl->_CreateOrGetBackgroundQueueByName(name, exceptionHandler);
    }

    bool QueueManager::IsExists(std::string const& name) const noexcept
    {
        return m_Impl->_IsExists(name);
    }

    bool QueueManager::IsMainExists() const noexcept
    {
        return IsExists(mainQueueName);
    }

    tQueueWeakPtr QueueManager::CreateOrGetMainQueue(tExceptionHandler const& exceptionHandler) const
    {
        return CreateOrGetBackgroundQueueByName(mainQueueName, exceptionHandler);
    }

    void QueueManager::ForgetByName(std::string const& name) const
    {
        m_Impl->_ForgetByName(name);
    }

    void QueueManager::ForgetMainQueue() const
    {
        ForgetByName(mainQueueName);
    }

    void QueueManager::KillAndForgetAll() const
    {
        m_Impl->_KillAndForgetAll();
    }

    QueueManager::QueueManager() noexcept
        : m_Impl(std::make_unique<_Impl>())
    {
    }

    std::string const QueueManager::mainQueueName = "Darkness.Concurrency.MainQueue";
} /// end namespace Darkness::Concurrency
