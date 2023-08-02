/*
 * CommandlineArguments.h
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

#ifndef COMMANDLINE_ARGUMENTS_H_
#define COMMANDLINE_ARGUMENTS_H_

#include "StringList.h"

enum class Mode
{
    CLANG_TIDY
};

Mode
modeFromString(const std::string& mode);
const char*
modeToString(Mode mode);

struct CommandlineArguments
{
public:
    CommandlineArguments(size_t argc, char const* const* argv);

    bool help = false;
    std::string self;

    Mode mode = Mode::CLANG_TIDY;

    std::string compilerDatabase;
    StringList sources;

    bool preprocess = false;
    bool quiet = false;

    std::string objectfile;
    std::string clangTidy;
    std::string ccache;

    StringList remainingArgs;

    static void printHelp();
};

#endif // COMMANDLINE_ARGUMENTS_H_
