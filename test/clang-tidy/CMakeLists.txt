#
# CMakeLists.txt
#
# Copyright (c) 2023 - 2024 Marius Zwicker
# All rights reserved.
#
# SPDX-License-Identifier: Apache-2.0
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

cmake_minimum_required(VERSION 3.25)
project(example CXX C)

set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_C_STANDARD 11)

# set(ENABLE_WARNING_AS_ERROR TRUE)
if( ENABLE_WARNING_AS_ERROR )
    set(CMAKE_COMPILE_WARNING_AS_ERROR TRUE)
endif()

foreach(INCL IN LISTS CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES)
    if( APPLE AND INCL MATCHES ".+c\\+\\+.+" )
        # workaround mixing clang-tidy versions on modern macOS failing
        # to find the C++ system includes by explicitly passing them
        add_compile_options(-isystem${INCL})
    endif()
endforeach()

if(MSVC)
    # test compiler specific flag handling
    add_compile_options(/EHs /EHc)
endif()

add_executable(hello_world
    hello_world.h
    hello_world.cpp
    main.cpp
)
if(COMMAND mz_auto_format)
    mz_auto_format(hello_world)
endif()

add_executable(hello_world_c
    main.c
)
if(COMMAND mz_auto_format)
    mz_auto_format(hello_world_c)
endif()
