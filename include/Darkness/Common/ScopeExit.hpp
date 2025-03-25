/// Project          Darkness. C++ library.
/// Copyright (c)    2024 Poturaiev Anton. All rights reserved.
///
/// @file    ScopeExit.hpp
/// @authors Poturaiev Anton
/// @license Distributed under the Boost Software License, Version 1.0.
///		     See accompanying file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
/// @brief   Declaration of the ScopeExit class

#pragma once

#include <functional>

namespace Darkness::Common {
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
} /// namespace Darkness::Common
