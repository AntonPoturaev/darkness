/// Project          Darkness. C++ library.
/// Copyright (c)    2024 Poturaiev Anton. All rights reserved.
///
/// @file    Spinlock.cpp
/// @authors Poturaiev Anton
/// @license Distributed under the Boost Software License, Version 1.0.
///		     See accompanying file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
/// @brief   Implementation of @class Spinlock

#include "Darkness/Concurrency/Spinlock.hpp"

namespace Darkness::Concurrency {
    Spinlock::Spinlock() noexcept
    {
        m_Flag.clear();
    }

    void Spinlock::lock() noexcept
    {
        while (m_Flag.test_and_set(std::memory_order_acquire)) {} /// spin
    }

    void Spinlock::unlock() noexcept
    {
        m_Flag.clear(std::memory_order_release);
    }
} /// namespace Darkness::Concurrency
