# Project          darkness. C++ library.
# Copyright (c)    2024 Poturaiev Anton. All rights reserved.
#
# @file    CMakeLists.txt
# @authors Poturaiev Anton
# @license Distributed under the Boost Software License, Version 1.0.
#		   See accompanying file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
# @brief   darkness project CMake configuration file.

cmake_minimum_required(VERSION 3.28)

project(darkness
    LANGUAGES CXX
    DESCRIPTION "The darkness is modern C++ library."
)

option(Enable_Timer_DEBUG "Turn this options to enable debug mode for AsyncTimer" ON)
option(Enable_Queue_DEBUG "Turn this options to enable debug mode for Queue" ON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(darkness_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

include_directories(${darkness_INCLUDE_DIR})

file(GLOB_RECURSE SOURCES
    ${darkness_INCLUDE_DIR}/*.hpp
    src/*.cpp
    src/*.hpp
)

add_library(${PROJECT_NAME} STATIC ${SOURCES})

if (Enable_Timer_DEBUG)
    target_compile_definitions(${PROJECT_NAME} PRIVATE -DDarkness_Concurrency_Timer_DEBUG)
endif()

if (Enable_Queue_DEBUG)
    target_compile_definitions(${PROJECT_NAME} PRIVATE -DDarkness_Concurrency_Queue_DEBUG)
endif()

target_include_directories(${PROJECT_NAME} PUBLIC ${darkness_INCLUDE_DIR})

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

if(CMAKE_SYSTEM_NAME STREQUAL Windows)
    set_property(TARGET ${PROJECT_NAME} PROPERTY COMPILE_WARNING_AS_ERROR ON)
endif()
