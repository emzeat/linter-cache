/*
 * test_Util.cpp
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

#include "Util.h"
#include "TemporaryFile.h"

TEST(Util, IsFile)
{
    ASSERT_FALSE(Util::is_file("/never/exists"));

    TemporaryFile temporary;
    temporary.writeText("foo");
    ASSERT_TRUE(Util::is_file(temporary.filename()));
    temporary.unlink();
    ASSERT_FALSE(Util::is_file(temporary.filename()));
}

TEST(Util, ReplaceAll)
{
    ASSERT_STREQ("foo-batz",
                 Util::replace_all("foobarbatz", "bar", "-").c_str());
    ASSERT_STREQ("foo1234567batz",
                 Util::replace_all("foobarbatz", "bar", "1234567").c_str());
    ASSERT_STREQ("foobarbatz",
                 Util::replace_all("foobarbatz", "world", "goo").c_str());
}
