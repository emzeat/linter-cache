/*
 * test_Logging.cpp
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

#include "Logging.h"
#include "TemporaryFile.h"
#include "Environment.h"

TEST(Logging, Debug)
{

    Environment env;
    env.set("LINTER_CACHE_DEBUG", 1);

    Logging logging;
    auto test_for_level = [&logging](Logging::Level level,
                                     const char* message) {
        testing::internal::CaptureStderr();
        LogMessage(level, logging).stream() << message;
        auto captured = testing::internal::GetCapturedStderr();
        ASSERT_NE(std::string::npos, captured.find(message));
    };
    test_for_level(Logging::Level::ERROR, "Some error");
    test_for_level(Logging::Level::WARNING, "Some warning");
    test_for_level(Logging::Level::INFO, "Note that!");
    test_for_level(Logging::Level::TRACE, "State is good");
}

TEST(Logging, Logfile)
{

    TemporaryFile logfile;

    Environment env;
    env.set("LINTER_CACHE_LOGFILE", logfile.filename());

    Logging logging;
    auto test_for_level = [&logging, &logfile](Logging::Level level,
                                               const char* message) {
        LogMessage(level, logging).stream() << message;
        auto captured = logfile.readText();
        ASSERT_NE(std::string::npos, captured.find(message));
    };
    test_for_level(Logging::Level::ERROR, "Some error");
    test_for_level(Logging::Level::WARNING, "Some warning");
    test_for_level(Logging::Level::INFO, "Note that!");
    test_for_level(Logging::Level::TRACE, "State is good");
}

TEST(Logging, None)
{
    Environment env;
    env.unset("LINTER_CACHE_LOGFILE");
    env.unset("LINTER_CACHE_DEBUG");

    Logging logging;
    auto test_for_level = [&logging](Logging::Level level,
                                     const char* message) {
        LogMessage(level, logging).stream() << message;
    };
    test_for_level(Logging::Level::ERROR, "Some error");
    test_for_level(Logging::Level::WARNING, "Some warning");
    test_for_level(Logging::Level::INFO, "Note that!");
    test_for_level(Logging::Level::TRACE, "State is good");
}
