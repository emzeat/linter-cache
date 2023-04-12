/*
 * test_SavedArguments.cpp
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

#include "SavedArguments.h"
#include "Environment.h"

TEST(SavedArguments, DefaultUseCase)
{
    Environment env;
    SavedArguments outer;

    outer.set("KEY", "value");
    outer.set("STRING", "Hello World!");
    outer.save(env);

    {
        SavedArguments inner;
        inner.load(env);

        ASSERT_STREQ("value", inner.get("KEY").c_str());
        ASSERT_STREQ("Hello World!", inner.get("STRING").c_str());
    }
}

TEST(SavedArguments, EscapeSymbols)
{
    Environment env;
    SavedArguments outer;

    outer.set("a=b", "value");
    outer.set("newline", "Three\nLines\nText");
    outer.save(env);

    {
        SavedArguments inner;
        inner.load(env);

        ASSERT_STREQ("value", inner.get("a=b").c_str());
        ASSERT_STREQ("Three\nLines\nText", inner.get("newline").c_str());
    }
}

TEST(SavedArguments, StringList)
{
    Environment env;
    SavedArguments outer;

    StringList values = {
        "one", "two\nthree", "four\"quote\"", "one\":\"element"
    };
    outer.set("a\"b", values);
    outer.set("dc", values);
    outer.save(env);

    {
        SavedArguments inner;
        inner.load(env);

        StringList defaultValue;
        ASSERT_EQ(values, inner.get("a\"b", defaultValue));
        ASSERT_EQ(values, inner.get("dc", defaultValue));
    }
}
