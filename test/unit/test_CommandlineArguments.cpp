/*
 * test_CommandlineArguments.cpp
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

#include "CommandlineArguments.h"

TEST(CommandlineArguments, None)
{
    std::vector<char const*> argv = { "cache-tidy" };

    CommandlineArguments args(argv.size(), argv.data());

    ASSERT_EQ(0, args.remainingArgs.size());
    ASSERT_EQ(0, args.sources.size());
    ASSERT_FALSE(args.help);
}

TEST(CommandlineArguments, Help)
{
    std::vector<char const*> argv = { "cache-tidy", "-h" };
    {
        CommandlineArguments args(argv.size(), argv.data());
        ASSERT_TRUE(args.help);
    }

    argv = { "cache-tidy", "--help" };
    {
        CommandlineArguments args(argv.size(), argv.data());
        ASSERT_TRUE(args.help);
    }
}

TEST(CommandlineArguments, CompileDatabase)
{
    std::vector<char const*> argv = { "cache-tidy", "-p", "build_dir" };

    {
        CommandlineArguments args(argv.size(), argv.data());
        ASSERT_STREQ("build_dir/compile_commands.json",
                     args.compilerDatabase.c_str());
        ASSERT_EQ(StringList({ "-p", "build_dir" }), args.remainingArgs);
    }

    argv = { "cache-tidy", "-p=build_dir" };
    {
        CommandlineArguments args(argv.size(), argv.data());
        ASSERT_STREQ("build_dir/compile_commands.json",
                     args.compilerDatabase.c_str());
        ASSERT_EQ(StringList({ "-p=build_dir" }), args.remainingArgs);
    }
}

TEST(CommandlineArguments, CCache)
{
    std::vector<char const*> argv = { "cache-tidy",
                                      "--linter-cache-ccache=ccaceh_v3" };

    CommandlineArguments args(argv.size(), argv.data());
    ASSERT_STREQ("ccaceh_v3", args.ccache.c_str());
}

TEST(CommandlineArguments, ClangTidy)
{
    std::vector<char const*> argv = {
        "cache-tidy", "--linter-cache-clang-tidy=hello_world",
        "foobar.cpp", "--config-file",
        "dummy.cfg",  "--export-fixes"
    };

    CommandlineArguments args(argv.size(), argv.data());
    ASSERT_EQ(Mode::CLANG_TIDY, args.mode);
    ASSERT_STREQ("hello_world", args.clangTidy.c_str());
    ASSERT_EQ(StringList({ "foobar.cpp" }), args.sources);
    ASSERT_EQ(StringList({ "--config-file", "dummy.cfg", "--export-fixes" }),
              args.remainingArgs);
    ASSERT_FALSE(args.preprocess);
}

TEST(CommandlineArguments, OutputExplicit)
{
    std::vector<char const*> argv = { "cache-tidy",
                                      "--linter-cache-clang-tidy=hello_world",
                                      "--linter-cache-o=temporary.stamp",
                                      "foobar.cpp" };

    CommandlineArguments args(argv.size(), argv.data());
    ASSERT_EQ(Mode::CLANG_TIDY, args.mode);
    ASSERT_STREQ("hello_world", args.clangTidy.c_str());
    ASSERT_EQ(StringList({ "foobar.cpp" }), args.sources);
    ASSERT_EQ(StringList(), args.remainingArgs);
    ASSERT_STREQ("temporary.stamp", args.objectfile.c_str());
    ASSERT_FALSE(args.preprocess);
}

TEST(CommandlineArguments, Output)
{
    std::vector<char const*> argv = { "cache-tidy",
                                      "--linter-cache-clang-tidy=hello_world",
                                      "-o=temporary.stamp",
                                      "foobar.cpp" };

    CommandlineArguments args(argv.size(), argv.data());
    ASSERT_EQ(Mode::CLANG_TIDY, args.mode);
    ASSERT_STREQ("hello_world", args.clangTidy.c_str());
    ASSERT_EQ(StringList({ "foobar.cpp" }), args.sources);
    ASSERT_EQ(StringList(), args.remainingArgs);
    ASSERT_STREQ("temporary.stamp", args.objectfile.c_str());
    ASSERT_FALSE(args.preprocess);
}

TEST(CommandlineArguments, Preprocess)
{
    std::vector<char const*> argv = { "cache-tidy", "-E", "-p", "_build" };

    CommandlineArguments args(argv.size(), argv.data());
    ASSERT_TRUE(args.preprocess);
}
