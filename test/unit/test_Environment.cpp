/*
 * test_Environment.cpp
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

#include "Environment.h"

TEST(Environment, Get)
{
    Environment env;

    auto value = env.get("PATH");
    ASSERT_FALSE(value.empty());

    value = env.get("NO_SUCH_VAR");
    ASSERT_TRUE(value.empty());
    value = env.get("NO_SUCH_VAR", "FALLBACK");
    ASSERT_FALSE(value.empty());
    ASSERT_STREQ("FALLBACK", value.c_str());
}

TEST(Environment, Set)
{
    Environment env;

    auto value = env.get("NO_SUCH_VAR");
    ASSERT_TRUE(value.empty());
    env.set("NO_SUCH_VAR", "foo");
    value = env.get("NO_SUCH_VAR");
    ASSERT_FALSE(value.empty());
    ASSERT_STREQ("foo", value.c_str());
    env.reset();
    value = env.get("NO_SUCH_VAR");
    ASSERT_TRUE(value.empty());
}

TEST(Environment, Integer)
{
    Environment env;

    auto value = env.get("TEST_INT_VAR", 1);
    ASSERT_EQ(value, 1);
    env.set("TEST_INT_VAR", 24);
    value = env.get("TEST_INT_VAR", 1);
    ASSERT_EQ(value, 24);
    env.reset();
    value = env.get("TEST_INT_VAR", 1);
    ASSERT_EQ(value, 1);
}

TEST(Environment, Double)
{
    Environment env;

    auto value = env.get("TEST_FLOAT_VAR", 1.0);
    ASSERT_EQ(value, 1.0);
    env.set("TEST_FLOAT_VAR", 23.456);
    value = env.get("TEST_FLOAT_VAR", 1.0);
    ASSERT_EQ(value, 23.456);
    env.reset();
    value = env.get("TEST_FLOAT_VAR", 1.0);
    ASSERT_EQ(value, 1.0);
}
