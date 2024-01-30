/*
 * Cache.cpp
 *
 * Copyright (c) 2022 - 2024 Marius Zwicker
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

#include <memory>
#include <iostream>

#include "Cache.h"
#include "Environment.h"
#include "Logging.h"
#include "Subprocess.h"
#include "TemporaryFile.h"
#include "Util.h"
#include "CompileCommands.h"

static constexpr char kEnvCcache[] = "CCACHE";
#ifdef MZ_WINDOWS
static constexpr char kPathSep[] = ";";
#else
static constexpr char kPathSep[] = ":";
#endif

Cache::Cache(const std::string& ccache, const Environment& env)
  : _ccache(ccache)
{
    if (_ccache.empty()) {
        _ccache = env.get(kEnvCcache, "ccache");
    }
    LOG(TRACE) << "Using ccache from '" << _ccache << "'";
}

void
Cache::execute(const CommandlineArguments& args,
               const Linter& linter,
               const std::string& objectfile,
               const std::string& sourcefile) const
{
    Environment env;

    // in order to work reliably we disable nodepend mode
    if(env.get("CCACHE_NODEPEND").empty() && env.get("CCACHE_DEPEND").empty()) {
        env.set("CCACHE_NODEPEND", "1");
    }

    if (Util::is_file(linter.executable())) {
        auto extraFiles = env.get("CCACHE_EXTRAFILES");
        if(!extraFiles.empty()) {
            extraFiles += kPathSep;
        }
        extraFiles += linter.executable();
        env.set("CCACHE_EXTRAFILES", extraFiles);
    }

    std::unique_ptr<NamedFile> temporary;
    if (objectfile.empty()) {
        temporary = std::make_unique<TemporaryFile>();
    } else {
        temporary = std::make_unique<NamedFile>(objectfile);
    }

    // ccache expects a regular compiler call here which is somewhat different
    // so we fake it and use a throw-away output mixed with the actual compiler
    // flags as given by the compile commands file. The actual arguments to
    // the linter will be restored later when ccache is invoking us again in
    // turn
    StringList ccacheArgs = { args.self };
    bool isMsvc = false;
    if (!args.compilerDatabase.empty()) {
        CompileCommands compilerDatabase(args.compilerDatabase);
        const auto flags = compilerDatabase.flagsForFile(sourcefile);
        ccacheArgs.insert(
          ccacheArgs.end(), flags.options.begin(), flags.options.end());

        isMsvc = (flags.compiler.find( "cl" ) != std::string::npos);
    }
    ccacheArgs.insert(ccacheArgs.end(),
                      { "-o", temporary->filename(), "-c", sourcefile });

    // we work like clang, force it unless overridden
    if(env.get("CCACHE_COMPILERTYPE").empty()) {
        env.set("CCACHE_COMPILERTYPE", isMsvc ? "clang-cl" : "clang");
    }

    try {
        invoke(ccacheArgs, args.quiet);
    } catch (ProcessError& error) {
        temporary->unlink();
        throw error;
    }
}

void
Cache::invoke(const StringList& args, bool quiet) const
{
    int flags = 0;
    if (quiet) {
        flags |= Process::CAPTURE_STDERR | Process::CAPTURE_STDOUT;
    }
    Process proc(_ccache + args, flags);
    LOG(TRACE) << "Cache: Running " << proc.cmd();
    try {
        proc.run();
    } catch (ProcessError& error) {
        std::cerr << proc.errorOutput();
        std::cout << proc.output();
        throw error;
    }
}
