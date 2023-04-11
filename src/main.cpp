/*
 * main.cpp
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

#include <memory>

#include "CommandlineArguments.h"
#include "Environment.h"
#include "Process.h"

#include "Cache.h"
#include "Linter.h"
#include "LinterClangTidy.h"
#include "Logging.h"

static constexpr char kMode[] = "Mode";

static std::unique_ptr<Linter>
createLinter(Mode mode,
             const CommandlineArguments& args,
             const Environment& env)
{
    switch (mode) {
        case Mode::CLANG_TIDY:
            LOG(TRACE) << "Linter is clang-tidy";
            return std::make_unique<LinterClangTidy>(args.clangTidy, env);
        default:
            throw ProcessError("Unknown operation mode", 1);
    }
}

static int
invokedFromCommandline(const CommandlineArguments& args, Environment& env)
{
    auto linter = createLinter(args.mode, args, env);
    Cache cache(args.ccache, env);

    for (const auto& source : args.sources) {
        SavedArguments saved;

        linter->prepare(source, args.remainingArgs, saved, env);
        saved.set(kMode, modeToString(args.mode));
        saved.save(env);

        cache.execute(args.self, args.objectfile, source, env);
    }

    return 0;
}

static int
invokedFromCcache(const SavedArguments& saved,
                  const CommandlineArguments& args,
                  Environment& env)
{
    auto linter = createLinter(modeFromString(saved.get(kMode)), args, env);

    NamedFile output(args.objectfile);
    if (args.preprocess) {
        linter->preprocess(saved, output);
    } else {
        linter->execute(saved, output);
    }

    return 0;
}

int
main(int argc, char* argv[])
{
    CommandlineArguments args(argc, argv);
    Environment env;
    LOG(TRACE) << "Invoked as " << StringList(argv, argc).join(" ");

    SavedArguments saved;
    saved.load(env);

    try {
        if (saved) {
            return invokedFromCcache(saved, args, env);
        }

        return invokedFromCommandline(args, env);
    } catch (ProcessError& error) {
        LOG(ERROR) << error.what();
        return error.exitCode();
    }
}
