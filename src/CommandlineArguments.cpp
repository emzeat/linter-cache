/*
 * CommandlineArguments.cpp
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
#include <algorithm>

#include "CommandlineArguments.h"

Mode
modeFromString(const std::string& mode)
{
    if (mode == "CLANG_TIDY") {
        return Mode::CLANG_TIDY;
    }
    throw std::runtime_error("No such Mode: " + mode);
}

const char*
modeToString(Mode mode)
{
    switch (mode) {
        case Mode::CLANG_TIDY:
            return "CLANG_TIDY";
    }
}

void
CommandlineArguments::printHelp()
{
    std::cout << "   Wrapper to invoke clang-tidy through ccache to accelerate "
                 "analysis as"
              << std::endl;
    std::cout << "   part of a regular compile job. Use as if running "
                 "clang-tidy directly."
              << std::endl;
    std::cout << std::endl;
    std::cout << "   Environment variables supported for configuration:"
              << std::endl;
    std::cout << "   {CLANG_TIDY_ENV}: Sets the clang-tidy executable."
              << std::endl;
    std::cout << "   {CCACHE_ENV}: Sets the ccache executable." << std::endl;
    std::cout << "   {CCACHE_TIDY_VERBOSE_ENV}: Enables debug messages."
              << std::endl;
    std::cout << "   {CCACHE_TIDY_LOGFILE_ENV}: Logs to the given file "
                 "(implies {CCACHE_TIDY_VERBOSE_ENV})"
              << std::endl;
    std::cout << std::endl;
    std::cout << "   Special flags supported to override configuration:"
              << std::endl;
    std::cout
      << "   --cache-tidy-o=<location of a stamp file to be touched on success>"
      << std::endl;
    std::cout
      << "   --cache-tidy-{CCACHE_ENV}=<location of the ccache executable>"
      << std::endl;
    std::cout << "   --cache-tidy-{CLANG_TIDY_ENV}=<location of the clang-tidy "
                 "executable>"
              << std::endl;
}

static bool
starts_with(const std::string& string, const std::string_view& substr)
{
    return 0 == string.compare(0, substr.size(), substr);
}

static bool
ends_with(const std::string& string, const std::string_view& substr)
{
    if (string.size() > substr.size()) {
        return 0 == string.compare(
                      string.size() - substr.size(), substr.size(), substr);
    }
    return false;
}

CommandlineArguments::CommandlineArguments(size_t argc, char const* const* argv)
{
    static constexpr std::string_view kCompileDb{ "-p=" };
    static constexpr std::string_view kOutput{ "-o=" };
    static constexpr std::string_view kLinterCache{ "--linter-cache_" };
    static constexpr std::string_view kCppExt{ ".cpp" };
    static constexpr std::string_view kCExt{ ".c" };

    remainingArgs.reserve(argc);
    self = argv[0];

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        // parsed_args.tidyargs.append(arg)
        if (arg == "-h" || arg == "--help") {
            printHelp();
            help = true;
            return;
        }
        if (arg == "-E") {
            preprocess = true;
            remainingArgs.push_back(arg);
        } else if (arg == "-p" && i + 1 < argc) {
            // path to compile db
            remainingArgs.push_back(arg);
            arg = argv[++i];
            remainingArgs.push_back(arg);
            compilerDatabase = arg + "/compile_commands.json";
        } else if (starts_with(arg, kCompileDb)) {
            // path to compile db
            remainingArgs.push_back(arg);
            compilerDatabase =
              arg.substr(kCompileDb.size()) + "/compile_commands.json";
        } else if (arg == "-o" && i + 1 < argc) {
            // path to write to
            arg = argv[++i];
            objectfile = arg;
        } else if (starts_with(arg, kOutput)) {
            // path to write to
            objectfile = arg.substr(kOutput.size());
        } else if (starts_with(arg, kLinterCache)) {
            // special linter-cache option
            arg = arg.substr(kLinterCache.size());

            static constexpr std::string_view kObjectFile = "o=";
            static constexpr std::string_view kCcache = "ccache=";
            static constexpr std::string_view kClangTidy = "clang-tidy=";
            if (starts_with(arg, kObjectFile)) {
                // output was explicitly requested
                objectfile = arg.substr(kObjectFile.size());
            } else if (starts_with(arg, kCcache)) {
                // ccache binary was overridden
                ccache = arg.substr(kCcache.size());
            } else if (starts_with(arg, kClangTidy)) {
                // clang-tidy binary was overridden
                clangTidy = arg.substr(kClangTidy.size());
                mode = Mode::CLANG_TIDY;
            }
        } else if (ends_with(arg, kCppExt) || ends_with(arg, kCExt)) {
            // sourcefile
            sources.push_back(arg);
        } else {
            remainingArgs.push_back(arg);
        }
    }
}
