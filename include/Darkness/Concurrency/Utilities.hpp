/// Project          Darkness. C++ library.
/// Copyright (c)    2024 Poturaiev Anton. All rights reserved.
///
/// @file    Utilities.hpp
/// @authors Poturaiev Anton
/// @license Distributed under the Boost Software License, Version 1.0.
///		     See accompanying file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
/// @brief   Declaration of some utilities for Concurrency

#pragma once

#include <Darkness/Concurrency/Types.hpp>

#include <string>
#include <thread>
#include <exception>

namespace Darkness::Concurrency {
    void SetThreadName(std::string const& name, std::thread::native_handle_type handle);

    void SetCurrentThreadName(std::string const& name);

    [[nodiscard]] std::thread::native_handle_type GetCurrentThreadHandle();

    void DebugExceptionHandler(std::exception_ptr const& exceptionPtr) noexcept;

    void AsyncCall(tTask task);
} /// namespace Darkness::Concurrency
