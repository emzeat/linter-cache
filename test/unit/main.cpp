/*
 * main.cpp
 *
 * Copyright (c) 2023 Marius Zwicker
 * All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <gtest/gtest.h>
#include <iostream>

#include "config.h"

#if LINTER_CACHE_HAVE_SETENV
// all good
#elif LINTER_CACHE_HAVE_SET_ENVIRONMENT_VARIABLE

    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
    #include <array>

inline void
setenv(const char* name, const char* value, int overwrite)
{
    static_cast<void>(overwrite);
    SetEnvironmentVariableA(name, value);
}

#endif

int
main(int argc, char* argv[])
{
    // This allows the user to override the flag on the command line.
    ::testing::InitGoogleTest(&argc, argv);

    // Make logging verbose to help with debugging
    setenv("LINTER_CACHE_DEBUG", "1", 1);

    return RUN_ALL_TESTS();
}
