#
# CMakeLists.txt
#
# Copyright (c) 2011 - 2023 Marius Zwicker
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

cmake_minimum_required(VERSION 3.1)
include(CheckSymbolExists)
include(CheckIncludeFile)
project(linter-cache CXX)

# build scripts
include(build/global.cmake)
include(build/macros.cmake)
include(build/conan.cmake)
include(build/linting.cmake)
set(CMAKE_CXX_STANDARD 17)

# check for platform features
check_symbol_exists( popen "stdio.h" LINTER_CACHE_HAVE_POPEN )
check_symbol_exists( mktemp "stdlib.h" LINTER_CACHE_HAVE_MKTEMP )
check_symbol_exists( getpid "unistd.h" LINTER_CACHE_HAVE_GETPID )
check_symbol_exists( unlink "unistd.h" LINTER_CACHE_HAVE_UNLINK )
check_symbol_exists( stat "sys/stat.h" LINTER_CACHE_HAVE_STAT )

# options
option(BUILD_LINTER_CACHE_TESTS "Enable testing of the linter-cache tool" ON)

# provide a config header with selected options and discovered features
configure_file(
    config.h.in
    ${CMAKE_BINARY_DIR}/include/config.h
)
include_directories(${CMAKE_BINARY_DIR}/include)

# static library of classes to allow for unit testing
add_library(linter-cache-obj STATIC
    src/Cache.h
    src/Cache.cpp
    src/Environment.h
    src/Environment.cpp
    src/Logging.h
    src/Logging.cpp
    src/TemporaryFile.h
    src/TemporaryFile.cpp
    src/Process.h
    src/Process.cpp
    src/SavedArguments.h
    src/SavedArguments.cpp
    src/StringList.h
    src/StringList.cpp
    src/CommandlineArguments.h
    src/CommandlineArguments.cpp
    src/Util.h
    src/Util.cpp

    src/Linter.h
    src/LinterClangTidy.h
    src/LinterClangTidy.cpp
)
target_include_directories(linter-cache-obj
    PUBLIC src
)
mz_target_props(linter-cache-obj)
mz_auto_format(linter-cache-obj)

# main program
add_executable(linter-cache
    src/main.cpp
)
target_link_libraries(linter-cache
    PRIVATE linter-cache-obj
)
mz_target_props(linter-cache)
mz_auto_format(linter-cache)
install(TARGETS linter-cache
    RUNTIME DESTINATION bin
)

# test coverage
if(BUILD_LINTER_CACHE_TESTS)
    enable_testing()

    find_package(GTest REQUIRED)
    include(GoogleTest)
    add_executable(linter-cache_tests
        test/unit/main.cpp
        test/unit/test_Environment.cpp
        test/unit/test_Logging.cpp
        test/unit/test_TemporaryFile.cpp
        test/unit/test_Process.cpp
        test/unit/test_SavedArguments.cpp
        test/unit/test_StringList.cpp
        test/unit/test_CommandlineArguments.cpp
    )
    target_link_libraries(linter-cache_tests
        GTest::GTest
        linter-cache-obj
    )
    mz_target_props(linter-cache_tests)
    mz_auto_format(linter-cache_tests)
    gtest_discover_tests(
        linter-cache_tests
        TEST_PREFIX unit.
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIRw}
    )

    find_program(PYTHON3 python3 python REQUIRED)
    find_program(CCACHE ccache REQUIRED)
    find_program(CLANG_TIDY clang-tidy REQUIRED)
    list(APPEND INTEGRATION_TESTS
        TestClangTidy.test_basic_run
        TestClangTidy.test_with_extra_args
        TestClangTidy.test_with_output
    )
    foreach(_test IN LISTS INTEGRATION_TESTS)
        add_test(
            NAME integration.${_test}
            COMMAND ${PYTHON3} test/clang-tidy/test.py ${_test}
                --base-dir ${CMAKE_BINARY_DIR}/test/clang-tidy
                --ccache ${CCACHE}
                --clang-tidy ${CLANG_TIDY}
                --linter-cache $<TARGET_FILE:linter-cache>
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        )
    endforeach()
endif()
