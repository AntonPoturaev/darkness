/// Project          Darkness. C++ library.
/// Copyright (c)    2024 Poturaiev Anton. All rights reserved.
///
/// @file    Utilities.cpp
/// @authors Poturaiev Anton
/// @license Distributed under the Boost Software License, Version 1.0.
///		     See accompanying file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
/// @brief   Implementation of some common utilities

#include "Darkness/Common/Utilities.hpp"

#include <format>
#include <cassert>
#include <iomanip>
#include <stdexcept>
#include <vector>
#include <chrono>

#if defined(_WIN32)

#include <Windows.h>

#elif defined(__linux__)
#include <cstdlib>
#include <memory>
/// etc...
#else
#error Unknown OS!
#endif

namespace Darkness::Common {
    namespace {
#if defined(_WIN32)
        inline namespace _win {
            std::wstring _Utf8toUtf16(char const* utf8Str, size_t utf8StrLength) noexcept(false)
            {
                if (utf8StrLength == 0)
                {
                    return {};
                }

                size_t const charsNeeded = ::MultiByteToWideChar(
                    CP_UTF8, 0, utf8Str, int(utf8StrLength)
                    , nullptr, 0);

                constexpr char const* errorMessage = "Failed string converting [UTF-8 -> UTF-16]";

                if (charsNeeded == 0)
                {
                    throw std::runtime_error(errorMessage);
                }

                std::vector<wchar_t> buffer(charsNeeded);
                auto const charsConverted = ::MultiByteToWideChar(
                    CP_UTF8, 0, utf8Str, int(utf8StrLength)
                    , &buffer[0], int(buffer.size()));

                if (charsConverted == 0)
                {
                    throw std::runtime_error(errorMessage);
                }

                return { buffer.data(), size_t(charsConverted) };
            }
        } /// end inline namespace _win

        namespace _os = _win;
#elif defined(__linux__)
        inline namespace _linux {
            std::wstring _Utf8toUtf16(char const* utf8Str, size_t utf8StrLength) noexcept(false)
            {
#pragma message("Implementation of _Utf8toUtf16 string conversion should be checked uder Linux!")

                auto buffer = std::make_unique<wchar_t[]>(utf8StrLength + 1);
                mbstowcs_s(nullptr, buffer.get(), utf8StrLength + 1, utf8Str, utf8StrLength);

                return buffer.get();
            }
        } /// end inline namespace _linux

        namespace _os = _linux;
#else
#error Unknown OS!
#endif

        std::runtime_error const g_UnknownException("Unknown exception!");

        template<typename MessageStr>
        std::string _MakeTraceExceptionMessage(MessageStr&& message) noexcept
        {
            static_assert(
                std::disjunction_v<
                    std::is_same<std::remove_reference_t<MessageStr>, char const*>
                    , std::is_same<std::remove_reference_t<MessageStr>, std::string const>
                                  >
                , "Wrong message string type!");

            return std::format("An exception was caught. Reason: \'{}\'", std::forward<MessageStr>(message));
        }
    } /// end unnamed namespace

    std::wstring Utf8toUtf16(char const* utf8Str, size_t utf8StrLength) noexcept(false)
    {
        return _os::_Utf8toUtf16(utf8Str, utf8StrLength);
    }

    std::wstring Utf8toUtf16(std::string_view const& utf8Str) noexcept(false)
    {
        return Utf8toUtf16(utf8Str.data(), utf8Str.size());
    }

    std::wstring Utf8toUtf16(std::string const& utf8Str) noexcept(false)
    {
        return Utf8toUtf16(utf8Str.c_str(), utf8Str.size());
    }

    std::exception const& UnknownException() noexcept
    {
        return g_UnknownException;
    }

    std::string MakeTraceExceptionMessage(char const* message) noexcept
    {
        return _MakeTraceExceptionMessage(message);
    }

    std::string MakeTraceExceptionMessage(std::string const& message) noexcept
    {
        return _MakeTraceExceptionMessage(message);
    }

    ScopeExit::ScopeExit(tCompletion completion) noexcept
        : m_Completion(std::move(completion))
    {
    }

    ScopeExit::~ScopeExit()
    {
        if (m_Completion)
        {
            m_Completion();
        }
    }
} /// namespace Darkness::Common
