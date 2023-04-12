/*
 * test_StringList.cpp
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

#include "StringList.h"

TEST(StringList, Construction)
{
    StringList empty;
    ASSERT_EQ(0, empty.size());

    StringList single("one");
    ASSERT_EQ(1, single.size());

    StringList multiple({ "one", "two", "three" });
    ASSERT_EQ(3, multiple.size());
}

TEST(StringList, Join)
{
    StringList list({ "one", "two", "three", "fold" });
    auto joined = list.join("/");

    ASSERT_STREQ("one/two/three/fold", joined.c_str());
}

TEST(StringList, Concat)
{
    StringList initial = { "cmd" };

    auto concat = initial + "arg0";
    StringList concatExpected0 = { "cmd", "arg0" };
    ASSERT_EQ(concatExpected0, concat);

    concat += "arg1";
    StringList concatExpected1 = { "cmd", "arg0", "arg1" };
    ASSERT_EQ(concatExpected1, concat);

    concat += StringList({ "arg2" });
    StringList concatExpected2 = { "cmd", "arg0", "arg1", "arg2" };
    ASSERT_EQ(concatExpected2, concat);

    auto concat2 = "pre" + initial;
    StringList concat2Expected = { "pre", "cmd" };
    ASSERT_EQ(concat2Expected, concat2);
}
