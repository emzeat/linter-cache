/*
 * LinterClangTidy.cpp
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

#include "LinterClangTidy.h"
#include "Process.h"
#include "Logging.h"
#include "CompileCommands.h"

static constexpr char kEnvClangTidy[] = "CLANG_TIDY";
static constexpr char kSaveSrc[] = "clangTidySrc";
static constexpr char kSaveArgs[] = "clangTidyArgs";
static constexpr char kSaveCompDb[] = "clangTidyCompDb";

LinterClangTidy::LinterClangTidy(const std::string& clangTidy,
                                 const Environment& env)
  : _clangTidy(clangTidy)
{
    if (_clangTidy.empty()) {
        _clangTidy = env.get(kEnvClangTidy, "clang-tidy");
    }
    LOG(TRACE) << "Using clang-tidy from '" << _clangTidy << "'";
}

void
LinterClangTidy::prepare(const std::string& sourceFile,
                         const CommandlineArguments& args,
                         SavedArguments& savedArgs,
                         Environment& env)
{
    // forward the initial args to clang-tidy
    savedArgs.set(kSaveSrc, sourceFile);
    savedArgs.set(kSaveArgs, args.remainingArgs);
    savedArgs.set(kSaveCompDb, args.compilerDatabase);

    // make sure to use our clang-tidy
    env.set(kEnvClangTidy, _clangTidy);
}

void
LinterClangTidy::preprocess(const SavedArguments& savedArgs,
                            std::string& output)
{
    // ccache wants to get the preproc output
    // we create this from
    // a) the source
    // b) the effective config
    // c) the effective lines in compdb

    auto sourcePath = savedArgs.get(kSaveSrc);
    NamedFile sourceFile(sourcePath);
    output += sourceFile.readText();
    output += invoke("--dump-config" + savedArgs.get(kSaveArgs, StringList()) +
                     savedArgs.get(kSaveSrc));

    CompileCommands compDb(savedArgs.get(kSaveCompDb));
    output += compDb.linesForFile(sourcePath).join("");
}

void
LinterClangTidy::execute(const SavedArguments& savedArgs, std::string& output)
{
    output =
      invoke(savedArgs.get(kSaveArgs, StringList()) + savedArgs.get(kSaveSrc));
}

std::string
LinterClangTidy::invoke(const StringList& args) const
{
    Process proc(_clangTidy + args);
    LOG(TRACE) << "Running " << proc.cmd();
    proc.run();
    return proc.output();
}