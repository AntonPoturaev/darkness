/// Project          Darkness. C++ library.
/// Copyright (c)    2024 Poturaiev Anton. All rights reserved.
///
/// @file    ScopeExit.cpp
/// @authors Poturaiev Anton
/// @license Distributed under the Boost Software License, Version 1.0.
///		     See accompanying file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
/// @brief   Implementation of the ScopeExit class

#include "Darkness/Common/ScopeExit.hpp"

namespace Darkness::Common {
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
