/*
 * test_CompileCommands.cpp
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

#include "CompileCommands.h"
#include "paths_in_tests.h"

static const StringList mainFlags = {
    "-isysroot",
    "/Applications/Xcode.app/Contents/Developer/Platforms/"
    "MacOSX.platform/Developer/SDKs/MacOSX.sdk",
    "-std=gnu++14"
};
static const std::string compiler =
  "/Applications/Xcode.app/Contents/Developer/Toolchains/"
  "XcodeDefault.xctoolchain/usr/bin/c++";

TEST(CompileCommands, MatchLinesRelative)
{
    CompileCommands db(kCompileCommandsJson);

    auto flags = db.flagsForFile("src/main.cpp");
    ASSERT_EQ(mainFlags, flags.options);
    ASSERT_EQ(compiler, flags.compiler);
}

TEST(CompileCommands, MatchLinesAbsolute)
{
    CompileCommands db(kCompileCommandsJson);

    auto flags = db.flagsForFile("/Volumes/Development/build/clang-ninja-debug/"
                                 "test/clang-tidy/src/main.cpp");
    ASSERT_EQ(mainFlags, flags.options);
    ASSERT_EQ(compiler, flags.compiler);
}
