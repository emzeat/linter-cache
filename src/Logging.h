/*
 * Logging.h
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

#ifndef LOGGING_H_
#define LOGGING_H_

#include <ostream>
#include <fstream>

class LogMessage;

class Logging
{
public:
    enum class Level
    {
        TRACE,
        INFO,
        WARNING,
        ERROR
    };

    Logging();

    std::ostream& stream(Level level);

    static Logging& defaultInstance();

private:
    std::ostream* _stream;
    std::ofstream _logfile;
};

class LogMessage
{
public:
    inline LogMessage(Logging::Level level, Logging& logging)
      : _stream(logging.stream(level))
    {}

    inline LogMessage(Logging::Level level)
      : LogMessage(level, Logging::defaultInstance())
    {}

    inline ~LogMessage() { _stream << std::endl; }

    inline std::ostream& stream() { return _stream; };

private:
    std::ostream& _stream;
};

#define LOG(level) LogMessage(Logging::Level::level).stream()

#define LOG_IF(level, condition)                                               \
    if (condition)                                                             \
    LogMessage(Logging::Level::##level).stream()

#endif // LOGGING_H_
