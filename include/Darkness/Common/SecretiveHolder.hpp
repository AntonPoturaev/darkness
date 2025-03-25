/// Project          Darkness. C++ library.
/// Copyright (c)    2025 Poturaiev Anton. All rights reserved.
///
/// @file    SecretiveHolder.hpp
/// @authors Poturaiev Anton
/// @license Distributed under the Boost Software License, Version 1.0.
///		     See accompanying file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
/// @brief   Implementation of the SecretiveHolder class.
///          This is utility class which present (aka) PIMPL idiom for uses into classes with dll interfaces which wille
///          be used with different c++ version e.g.: 17 and 23. This prevent a case [field of class type without a DLL
///          interface used in a class with a DLL interface], where options COMPILE_WARNING_AS_ERROR is on.

#pragma once

#include <memory>

namespace Darkness::Common {
    template<typename T>
    class SecretiveHolder
    {
        using tDataHolder = size_t;

    public:
        using value_type = T;

    private:
        using tStorage = std::unique_ptr<value_type>;
        static_assert(sizeof(value_type) == sizeof(tDataHolder), "Wrong DataHolder size!");

    public:
        SecretiveHolder()
        {
            _Storage() = std::make_unique<value_type>();
        }

        template<typename... Args>
        explicit SecretiveHolder(Args&& ...args)
        {
            _Storage() = std::make_unique<value_type>(std::forward<Args>(args)...);
        }

        ~SecretiveHolder()
        {
            _Storage().~tStorage();
        }

        [[nodiscard]] constexpr value_type& GetObject() noexcept
        {
            return *_Storage();
        }

        [[nodiscard]] constexpr value_type const& GetObject() const noexcept
        {
            return *_Storage();
        }

    private:
        constexpr tStorage& _Storage() noexcept
        {
            return reinterpret_cast<tStorage&>(m_DataHolder);
        }

    private:
        tDataHolder m_DataHolder {};
    };
} /// namespace Darkness::Common
