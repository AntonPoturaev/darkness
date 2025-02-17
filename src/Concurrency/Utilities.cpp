/// Project          Darkness. C++ library.
/// Copyright (c)    2024 Poturaiev Anton. All rights reserved.
///
/// @file    ThreadName.cpp
/// @authors Poturaiev Anton
/// @license Distributed under the Boost Software License, Version 1.0.
///		     See accompanying file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
/// @brief   Implementation of some utilities for menage ThreadName

#include <Darkness/Concurrency/Utilities.hpp>
#include "Darkness/Common/Utilities.hpp"

#include <iostream>

#if defined(_WIN32)

#include <Windows.h>
#include <processthreadsapi.h>

#elif defined(__linux__)
#include <pthread.h>
#include <cstdlib>
#include <memory>
/// etc...
#else
#error Unknown OS!
#endif

namespace Darkness::Concurrency {
    namespace {
#if defined(_WIN32)
        inline namespace _win {
            void _SetThreadName(std::string const& name, std::thread::native_handle_type handle)
            {
                [[maybe_unused]] HRESULT const result = ::SetThreadDescription(handle, Common::Utf8toUtf16(name).c_str());
                //(void) result; /// @debug Uncomment for debug view
            }

            std::thread::native_handle_type _GetCurrentThread()
            {
                return ::GetCurrentThread();
            }
        } /// end inline namespace _win

        namespace _os = _win;
#elif defined(__linux__)
        inline namespace _linux {
            void _SetThreadName(std::string const& name, std::thread::native_handle_type handle) noexcept
            {
                pthread_setname_np(handle, name.c_str());
            }

            std::thread::native_handle_type _GetCurrentThread() noexcept
            {
                return pthread_self();
            }
        } /// end inline namespace _linux

        namespace _os = _linux;
#else
#error Unknown OS!
#endif
    } /// end unnamed namespace

    void SetThreadName(std::string const& name, std::thread::native_handle_type handle)
    {
        _os::_SetThreadName(name, handle);
    }

    void SetCurrentThreadName(std::string const& name)
    {
        SetThreadName(name, GetCurrentThreadHandle());
    }

    std::thread::native_handle_type GetCurrentThreadHandle()
    {
        return _os::_GetCurrentThread();
    }

    void DebugExceptionHandler(std::exception_ptr const& exceptionPtr) noexcept
    {
        try
        {
            if (exceptionPtr)
            {
                std::rethrow_exception(exceptionPtr);
            }
        }
        catch (std::exception const& exception)
        {
            std::cerr << "Exception: " << exception.what() << '\n';
        }
        catch (...)
        {
            std::cerr << Common::UnknownException().what() << '\n';
        }
    }

    void AsyncCall(tTask task)
    {
        if (task)
        {
            std::jthread([task = std::move(task)]() { task(); }).detach();
        }
    }
} /// namespace Darkness::Concurrency
