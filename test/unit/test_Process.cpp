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
#include "custom_main.h"

// all of the tests make use of the fact that cmake has to
// be available as we use it as a build system in the first place

TEST(Process, RunSuccess)
{
    Process process({ kCustomMainPath });
    ASSERT_NO_THROW(process.run());
}

TEST(Process, RunFromPathEnv)
{
    Process process({ "cmake", "--version" });
    ASSERT_NO_THROW(process.run());
}

TEST(Process, RunWithArgs)
{
    Process process({ kCustomMainPath, "--exit", "0" });
    ASSERT_NO_THROW(process.run());
}

TEST(Process, RunFailure)
{
    Process process({ kCustomMainPath, "--exit", "2" });
    ASSERT_THROW(process.run(), ProcessError);
}

TEST(Process, CaptureStdOut)
{
    static constexpr char kOutput[] = "Hello World!";

    Process process({ kCustomMainPath, "--stdout", kOutput });
    testing::internal::CaptureStdout();
    testing::internal::CaptureStderr();
    ASSERT_NO_THROW(process.run());
    auto const capturedStdout = testing::internal::GetCapturedStdout();
    ASSERT_TRUE(capturedStdout.empty()) << capturedStdout;
    auto const capturedStderr = testing::internal::GetCapturedStderr();
    ASSERT_TRUE(capturedStderr.empty()) << capturedStderr;
    ASSERT_STREQ(kOutput, process.output().c_str());
    ASSERT_TRUE(process.errorOutput().empty()) << process.errorOutput();
}

TEST(Process, CaptureStdErr)
{
    static constexpr char kOutput[] = "Hello World!";

    Process process({ kCustomMainPath, "--stderr", kOutput });
    testing::internal::CaptureStdout();
    testing::internal::CaptureStderr();
    ASSERT_NO_THROW(process.run());
    auto const capturedStdout = testing::internal::GetCapturedStdout();
    ASSERT_TRUE(capturedStdout.empty()) << capturedStdout;
    auto const capturedStderr = testing::internal::GetCapturedStderr();
    ASSERT_TRUE(capturedStderr.empty()) << capturedStderr;
    ASSERT_STREQ(kOutput, process.errorOutput().c_str());
    ASSERT_TRUE(process.output().empty()) << process.output();
}

TEST(Process, CaptureBoth)
{
    static constexpr char kOutput[] = "Some info message :)";
    static constexpr char kErrput[] = "Oh my !!";

    Process process({ kCustomMainPath,
                      "--stdout",
                      kOutput,
                      "--sleep",
                      "2",
                      "--stderr",
                      kErrput });
    testing::internal::CaptureStdout();
    testing::internal::CaptureStderr();
    ASSERT_NO_THROW(process.run());
    auto const capturedStdout = testing::internal::GetCapturedStdout();
    ASSERT_TRUE(capturedStdout.empty()) << capturedStdout;
    auto const capturedStderr = testing::internal::GetCapturedStderr();
    ASSERT_TRUE(capturedStderr.empty()) << capturedStderr;
    ASSERT_STREQ(kOutput, process.output().c_str());
    ASSERT_STREQ(kErrput, process.errorOutput().c_str());
}

TEST(Process, ForwardStdout)
{
    static constexpr char kOutput[] = "Hello World!";

    Process process({ kCustomMainPath, "--stdout", kOutput },
                    Process::Flags::FORWARD_OUTPUT);
    testing::internal::CaptureStdout();
    testing::internal::CaptureStderr();
    ASSERT_NO_THROW(process.run());
    auto const capturedStdout = testing::internal::GetCapturedStdout();
    ASSERT_STREQ(kOutput, capturedStdout.c_str());
    auto const capturedStderr = testing::internal::GetCapturedStderr();
    ASSERT_TRUE(capturedStderr.empty()) << capturedStderr;
    ASSERT_TRUE(process.output().empty()) << process.output();
    ASSERT_TRUE(process.errorOutput().empty()) << process.errorOutput();
}

TEST(Process, ForwardStderr)
{
    static constexpr char kOutput[] = "Hello World!";

    Process process({ kCustomMainPath, "--stderr", kOutput },
                    Process::Flags::FORWARD_OUTPUT);
    testing::internal::CaptureStderr();
    testing::internal::CaptureStdout();
    ASSERT_NO_THROW(process.run());
    auto const capturedStdout = testing::internal::GetCapturedStdout();
    ASSERT_TRUE(capturedStdout.empty()) << capturedStdout;
    auto const capturedStderr = testing::internal::GetCapturedStderr();
    ASSERT_TRUE(capturedStderr.empty()) << capturedStderr;
    ASSERT_TRUE(process.output().empty()) << process.output();
    ASSERT_TRUE(process.errorOutput().empty()) << process.errorOutput();
}

TEST(Process, ForwardBoth)
{
    static constexpr char kOutput[] = "Some info message :)";
    static constexpr char kErrput[] = "Oh my !!";

    Process process(
      { kCustomMainPath, "--stderr", kErrput, "--stdout", kOutput },
      Process::Flags::FORWARD_OUTPUT);
    testing::internal::CaptureStderr();
    testing::internal::CaptureStdout();
    ASSERT_NO_THROW(process.run());
    auto const capturedStdout = testing::internal::GetCapturedStdout();
    ASSERT_STREQ(kOutput, capturedStdout.c_str());
    auto const capturedStderr = testing::internal::GetCapturedStderr();
    ASSERT_STREQ(kErrput, capturedStderr.c_str());
    ASSERT_TRUE(process.output().empty()) << process.output();
    ASSERT_TRUE(process.errorOutput().empty()) << process.errorOutput();
}

TEST(Process, CaptureReallyLong)
{
    static constexpr size_t kOutputLen = 10240;
    std::string output;
    output.reserve(kOutputLen);
    for (size_t i = 0; i < kOutputLen; ++i) {
        output.push_back(static_cast<char>('a' + (i % 26)));
    }

    Process process({ kCustomMainPath, "--stdout", output });
    ASSERT_NO_THROW(process.run());
    ASSERT_EQ(output.size(), process.output().size());
    ASSERT_EQ(output, process.output());
}
