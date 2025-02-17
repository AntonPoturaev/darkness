/// Project          Darkness. C++ library.
/// Copyright (c)    2024 Poturaiev Anton. All rights reserved.
///
/// @file    Utilities.hpp
/// @authors Poturaiev Anton
/// @license Distributed under the Boost Software License, Version 1.0.
///		     See accompanying file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
/// @brief   Declaration of some common utilities

#pragma once

#include <string>
#include <string_view>
#include <exception>
#include <functional>
#include <type_traits>

namespace Darkness::Common {
    [[nodiscard]] std::wstring Utf8toUtf16(char const* utf8Str, size_t utf8StrLength) noexcept(false);

    [[nodiscard]] std::wstring Utf8toUtf16(std::string_view const& utf8Str) noexcept(false);

    [[nodiscard]] std::wstring Utf8toUtf16(std::string const& utf8Str) noexcept(false);

    [[nodiscard]] std::exception const& UnknownException() noexcept;

    std::string MakeTraceExceptionMessage(char const* message) noexcept;

    std::string MakeTraceExceptionMessage(std::string const& message) noexcept;

    class ScopeExit final
    {
    public:
        using tCompletion = std::function<void()>;

    public:
        explicit ScopeExit(tCompletion completion) noexcept;

        ~ScopeExit();

    private:
        tCompletion const m_Completion;
    };

    inline void HashCombine(std::size_t& seed) {}

    template<typename T, typename... Rest>
    void HashCombine(std::size_t& seed, T const& v, Rest&& ... rest)
    {
        constexpr std::hash<T> const hash {};
        seed ^= hash(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        (HashCombine(seed, std::forward<Rest>(rest)), ...);
    }
} /// namespace Darkness::Common
