/*
 * test_NamedFile.cpp
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

#include "NamedFile.h"

static std::string
generateText(std::size_t length)
{
    std::string ret;
    ret.reserve(length);
    for (std::size_t i = 0; i < length; ++i) {
        ret.push_back(static_cast<char>('a' + (i % 52)));
    }
    return ret;
}

TEST(NamedFile, WriteText)
{
    auto const testedText = generateText(65000);

    NamedFile named("unittest.txt");
    ASSERT_TRUE(named);
    ASSERT_TRUE(named.writeText(testedText));

    {
        std::ifstream stream(named.filename());
        stream.exceptions(std::ios::badbit | std::ios::failbit);
        ASSERT_TRUE(stream.is_open());
        std::string text;
        stream >> text;
        EXPECT_STREQ(testedText.c_str(), text.c_str());
    }

    named.unlink();
}

TEST(NamedFile, ReadText)
{
    auto const testedText = generateText(32578);

    NamedFile named("unittest.txt");
    ASSERT_TRUE(named);
    ASSERT_TRUE(named.readText().empty());

    {
        std::ofstream stream(named.filename());
        ASSERT_TRUE(stream.is_open());
        stream << testedText;
        stream.close();
    }

    EXPECT_STREQ(testedText.c_str(), named.readText().c_str());
    named.unlink();
}

TEST(NamedFile, ReadLines)
{
    StringList const testedLines = { generateText(128),
                                     generateText(2400),
                                     generateText(65783) };

    NamedFile named("unittest.txt");
    {
        std::ofstream stream(named.filename());
        ASSERT_TRUE(stream.is_open());
        stream << testedLines.join('\n');
        stream.close();
    }

    auto lines = named.readLines();
    EXPECT_EQ(lines.size(), testedLines.size());
    for (std::size_t i = 0;
         i < testedLines.size() && i < lines.size() && !HasFailure();
         ++i) {
        EXPECT_STREQ(testedLines[i].c_str(), lines[i].c_str());
    }

    named.unlink();
}
