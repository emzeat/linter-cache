/*
 * test_TemporaryFile.cpp
 *
 * Copyright (c) 2022 - 2023 Marius Zwicker
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
#include <fstream>

#include "TemporaryFile.h"

TEST(TemporaryFile, Write)
{

    TemporaryFile temporary;
    ASSERT_TRUE(temporary);
    ASSERT_TRUE(temporary.writeText("Unittest"));

    std::ifstream stream(temporary.filename());
    stream.exceptions(std::ios::badbit | std::ios::failbit);
    ASSERT_TRUE(stream.is_open());
    std::string text;
    stream >> text;
    EXPECT_STREQ("Unittest", text.c_str());
}

TEST(TemporaryFile, Read)
{
    TemporaryFile temporary;
    ASSERT_TRUE(temporary);
    ASSERT_TRUE(temporary.readText().empty());

    std::ofstream stream(temporary.filename());
    ASSERT_TRUE(stream.is_open());
    stream << "Unittest";
    stream.close();

    ASSERT_STREQ("Unittest", temporary.readText().c_str());
}
