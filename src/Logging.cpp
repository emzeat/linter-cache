/*
 * Logging.cpp
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

#include <iostream>

#include "Logging.h"
#include "Environment.h"

Logging&
Logging::defaultInstance()
{
    static Logging s_instance;
    return s_instance;
}

Logging::Logging()
  : _stream(nullptr)
  , _logfile()
{
    Environment env;

    auto logfile =
      env.get("LINTER_CACHE_LOGFILE", env.get("CACHE_TIDY_LOGFILE"));
    auto debug =
      env.get("LINTER_CACHE_DEBUG", env.get("CACHE_TIDY_VERBOSE", false));
    if (!logfile.empty()) {
        _logfile.open(logfile, std::ios::out | std::ios::app);
        _stream = &_logfile;
    } else if (debug) {
        _stream = &std::cerr;
    } else {
        // FIXME(zwicker): Optimize this better
        _logfile.setstate(std::ios::badbit);
        _stream = &_logfile;
    }
}

std::ostream&
Logging::stream(Level level)
{
    switch (level) {
        case Level::TRACE:
            *_stream << "TRACE: ";
            break;
        case Level::INFO:
            *_stream << " INFO: ";
            break;
        case Level::WARNING:
            *_stream << " WARN: ";
            break;
        case Level::ERROR:
            *_stream << "  ERR: ";
            break;
    }
    return *_stream;
}
