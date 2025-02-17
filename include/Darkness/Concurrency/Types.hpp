/// Project          Darkness. C++ library.
/// Copyright (c)    2024 Incoresoft. All rights reserved.
///
/// @file    Types.hpp
/// @authors Poturaiev Anton
/// @date    13.07.2024
/// @brief   Definition of basic types for Darkness.Concurrency module

#pragma once

#include <functional>
#include <exception>

namespace Darkness::Concurrency {
    using tTask = std::function<void()>;
    using tExceptionHandler = std::function<void(std::exception_ptr)>;

    enum class eAsyncState
    {
        Free
        , Busy
        , Stopping
        , Stopped
    };
} /// end namespace Darkness::Concurrency
