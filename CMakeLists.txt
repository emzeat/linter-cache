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

cmake_minimum_required(VERSION 3.25)
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
check_symbol_exists( _popen "stdio.h" LINTER_CACHE_HAVE__POPEN )
check_symbol_exists( close "unistd.h" LINTER_CACHE_HAVE_CLOSE )
check_symbol_exists( mkstemp "stdlib.h" LINTER_CACHE_HAVE_MKSTEMP )
check_symbol_exists( getpid "unistd.h" LINTER_CACHE_HAVE_GETPID )
check_symbol_exists( unlink "unistd.h" LINTER_CACHE_HAVE_UNLINK )
check_symbol_exists( execvp "unistd.h" LINTER_CACHE_HAVE_EXECVP )
check_symbol_exists( SYS_pidfd_open "sys/syscall.h" LINTER_CACHE_HAVE_PIDFD_OPEN )
check_symbol_exists( kevent "sys/event.h" LINTER_CACHE_HAVE_KEVENT )
check_symbol_exists( stat "sys/stat.h" LINTER_CACHE_HAVE_STAT )
check_symbol_exists( getenv "stdlib.h" LINTER_CACHE_HAVE_GETENV )
check_symbol_exists( setenv "stdlib.h" LINTER_CACHE_HAVE_SETENV )
check_symbol_exists( unsetenv "stdlib.h" LINTER_CACHE_HAVE_UNSETENV )
check_symbol_exists( SetEnvironmentVariableA "Windows.h" LINTER_CACHE_HAVE_SET_ENVIRONMENT_VARIABLE )
check_symbol_exists( GetEnvironmentVariableA "Windows.h" LINTER_CACHE_HAVE_GET_ENVIRONMENT_VARIABLE )
check_symbol_exists( GetCurrentProcessId "Windows.h" LINTER_CACHE_HAVE_GET_CURRENT_PROCESS_ID )
check_symbol_exists( GetTempFileNameA "Windows.h" LINTER_CACHE_HAVE_GET_TEMP_FILE_NAME )
check_symbol_exists( DeleteFileA "Windows.h" LINTER_CACHE_HAVE_DELETE_FILE )
check_symbol_exists( GetFileAttributesA "Windows.h" LINTER_CACHE_HAVE_GET_FILE_ATTRIBUTES )
check_symbol_exists( GetFullPathNameA "Windows.h" LINTER_CACHE_HAVE_GET_FULL_PATHNAME )
check_symbol_exists( CreateProcessA "Windows.h" LINTER_CACHE_HAVE_CREATE_PROCESS )

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
    src/Cache.cpp
    src/Cache.h
    src/CommandlineArguments.cpp
    src/CommandlineArguments.h
    src/CompileCommands.cpp
    src/CompileCommands.h
    src/Environment.cpp
    src/Environment.h
    src/Logging.cpp
    src/Logging.h
    src/NamedFile.cpp
    src/NamedFile.h
    src/SavedArguments.cpp
    src/SavedArguments.h
    src/StringList.cpp
    src/StringList.h
    src/Subprocess.cpp
    src/Subprocess.h
    src/TemporaryFile.cpp
    src/TemporaryFile.h
    src/Util.cpp
    src/Util.h

    src/Linter.h
    src/LinterClangTidy.h
    src/LinterClangTidy.cpp
)
target_include_directories(linter-cache-obj
    PUBLIC src
)
if(LINTER_CACHE_HAVE_EXECVP AND (LINTER_CACHE_HAVE_PIDFD_OPEN OR LINTER_CACHE_HAVE_KEVENT))
    target_sources(linter-cache-obj
        PRIVATE src/Subprocess_fork.cpp
    )
elseif(LINTER_CACHE_HAVE_CREATE_PROCESS)
    target_sources(linter-cache-obj
        PRIVATE src/Subprocess_createprocess.cpp
    )
elseif(LINTER_CACHE_HAVE_POPEN OR LINTER_CACHE_HAVE__POPEN)
    target_sources(linter-cache-obj
        PRIVATE src/Subprocess_popen.cpp
    )
endif()
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
if(BUILD_LINTER_CACHE_TESTS AND NOT CONAN_EXPORTED)
    enable_testing()

    find_package(GTest REQUIRED)
    include(GoogleTest)
    add_executable(custom_main
        test/unit/custom_main.cpp
    )
    mz_target_props(custom_main)
    mz_auto_format(custom_main)
    file(GENERATE
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/test/unit/custom_main.h
        INPUT ${CMAKE_CURRENT_SOURCE_DIR}/test/unit/custom_main.h.in
    )
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/test/unit/paths_in_tests.h.in
        ${CMAKE_CURRENT_BINARY_DIR}/test/unit/paths_in_tests.h
    )
    add_executable(linter-cache_tests
        test/unit/main.cpp
        test/unit/test_Environment.cpp
        test/unit/test_CompileCommands.cpp
        test/unit/test_Logging.cpp
        test/unit/test_TemporaryFile.cpp
        test/unit/test_NamedFile.cpp
        test/unit/test_SavedArguments.cpp
        test/unit/test_StringList.cpp
        test/unit/test_Subprocess.cpp
        test/unit/test_CommandlineArguments.cpp
        test/unit/test_Util.cpp
    )
    target_link_libraries(linter-cache_tests
        GTest::GTest
        linter-cache-obj
    )
    target_include_directories(linter-cache_tests
        PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/test/unit
    )
    mz_target_props(linter-cache_tests)
    mz_auto_format(linter-cache_tests)
    gtest_discover_tests(
        linter-cache_tests
        TEST_PREFIX unit.
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )

    add_subdirectory(test/clang-tidy)
    find_program(PYTHON3 python3 python REQUIRED)
    unset(CCACHE CACHE)
    find_package(ccache REQUIRED)
    find_program(CCACHE ccache PATHS ${ccache_PACKAGE_FOLDER_RELEASE}/bin ${ccache_PACKAGE_FOLDER_DEBUG}/bin NO_DEFAULT_PATH REQUIRED)
    find_program(CLANG_TIDY clang-tidy REQUIRED)
    list(APPEND INTEGRATION_TESTS
        TestClangTidy.test_source_modification
        TestClangTidy.test_source_modification_c
        TestClangTidy.test_include_modification
        TestClangTidy.test_config_modification
        TestClangTidy.test_param_modification
        TestClangTidy.test_with_extra_args
        TestClangTidy.test_with_output_file
        TestClangTidy.test_error_logging
        TestClangTidy.test_with_different_directories
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
