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

    // true when help printing was requested
    bool help = false;

    // the name by which the linter cache was invoked
    std::string self;

    // the mode in which the linter cache is running
    Mode mode = Mode::CLANG_TIDY;

    // detail on a compiler database passed (if any)
    std::string compilerDatabase;

    // the sources passed for linting
    StringList sources;

    // true when invoked with -E to get preprocessing output
    bool preprocess = false;

    // true when invoked with --quiet to silence output
    bool quiet = false;

    // any object file specified via -o
    std::string objectfile;

    // the path to clang-tidy as given via `--clang-tidy`
    std::string clangTidy;

    // the path to ccache as given via `--ccache`
    std::string ccache;

    // arguments not matching to any of the above and
    // which hence need to be forwarded to the linter
    StringList remainingArgs;

    // will print an overview on available options
    static void printHelp();
};

#endif // COMMANDLINE_ARGUMENTS_H_
