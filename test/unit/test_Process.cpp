/*
 * test_Process.cpp
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

#include "Process.h"

// all of the tests make use of the fact that cmake has to
// be available as we use it as a build system in the first place

TEST(Process, Run)
{
    Process process({ "cmake" });
    ASSERT_NO_THROW(process.run());
}

TEST(Process, RunWithArgs)
{
    Process process({ "cmake", "--version" });
    ASSERT_NO_THROW(process.run());
    auto output = process.output();
    ASSERT_NE(std::string::npos, output.find("cmake version"));
}

TEST(Process, RunFailure)
{
    Process process({ "cmake", "foo" });
    ASSERT_THROW(process.run(), ProcessError);
}

TEST(Process, CaptureOutput)
{
    Process process({ "cmake", "--version" });
    testing::internal::CaptureStdout();
    ASSERT_NO_THROW(process.run());
    std::string forwarded = testing::internal::GetCapturedStdout();
    ASSERT_TRUE(forwarded.empty());
}

TEST(Process, ForwardOutput)
{
    Process process({ "cmake", "--version" }, Process::Flags::FORWARD_OUTPUT);
    testing::internal::CaptureStdout();
    ASSERT_NO_THROW(process.run());
    std::string forwarded = testing::internal::GetCapturedStdout();
    ASSERT_NE(std::string::npos, forwarded.find("cmake version"));
}
