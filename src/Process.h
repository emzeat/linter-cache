/*
 * Process.h
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

#ifndef PROCESS_H_
#define PROCESS_H_

#include <string>
#include <stdexcept>

#include "StringList.h"

class ProcessError : public std::runtime_error
{
public:
    inline ProcessError(const std::string& cmd, int exitCode)
      : std::runtime_error(cmd)
      , _exitCode(exitCode)
    {}

    inline int exitCode() const { return _exitCode; }
    inline std::string cmd() const { return what(); }

private:
    int _exitCode;
};

class Process
{
public:
    Process(const StringList& cmd);
    Process(const char* cmd);

    inline const std::string& cmd() const { return _cmd; }

    inline const std::string& output() const { return _output; }

    inline int exitCode() const { return _exitCode; }

    void run();

private:
    std::string _cmd;
    std::string _output;
    int _exitCode;
};

#endif // ENVIRONMENT_H_
