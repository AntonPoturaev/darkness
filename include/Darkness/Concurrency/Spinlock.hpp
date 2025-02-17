/// Project          Darkness. C++ library.
/// Copyright (c)    2024 Poturaiev Anton. All rights reserved.
///
/// @file    Spinlock.hpp
/// @authors Poturaiev Anton
/// @license Distributed under the Boost Software License, Version 1.0.
///		     See accompanying file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
/// @brief   Declaration of @class Spinlock

#pragma once

#include <atomic>

namespace Darkness::Concurrency {
    class Spinlock
    {
    public:
        Spinlock() noexcept;

        Spinlock(Spinlock const&) noexcept = delete;

        Spinlock(Spinlock&&) noexcept = delete;

        Spinlock& operator=(Spinlock const&) noexcept = delete;

        Spinlock& operator=(Spinlock&&) noexcept = delete;

        void lock() noexcept;

        void unlock() noexcept;

    private:
        std::atomic_flag m_Flag;
    };
} /// namespace Darkness::Concurrency
