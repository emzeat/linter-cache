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
#include <iostream>

#include "CommandlineArguments.h"
#include "Environment.h"
#include "Subprocess.h"

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

        linter->prepare(source, args, saved, env);
        saved.set(kMode, modeToString(args.mode));
        saved.save(env);

        cache.execute(args, *linter, args.objectfile, source);
    }

    return 0;
}

static int
invokedFromCcache(const SavedArguments& saved,
                  const CommandlineArguments& args,
                  Environment& env)
{
    auto linter = createLinter(modeFromString(saved.get(kMode)), args, env);

    std::string output;
    if (args.preprocess) {
        linter->preprocess(saved, output);
        if (args.objectfile.empty()) {
            LOG(TRACE) << "Preprocessing to stdout:\n" << output;
            std::cout << output << std::endl;
        } else {
            LOG(TRACE) << "Preprocessing to '" << args.objectfile << "':\n"
                       << output;
            NamedFile objectfile(args.objectfile);
            objectfile.writeText(output);
        }
    } else {
        linter->execute(saved, output);
        if (!args.objectfile.empty()) {
            LOG(TRACE) << "Linting to '" << args.objectfile << "':\n" << output;
            NamedFile objectfile(args.objectfile);
            objectfile.writeText(output);
        }
    }

    return 0;
}

int
main(int argc, char* argv[]) // NOLINT(bugprone-exception-escape)
{
    try {
        CommandlineArguments args(argc, argv);
        Environment env;
        LOG(TRACE) << "Invoked as " << StringList(argv, argc);

        SavedArguments saved;
        saved.load(env);

        if (saved) {
            return invokedFromCcache(saved, args, env);
        }

        return invokedFromCommandline(args, env);
    } catch (ProcessError& error) {
        LOG(ERROR) << "ProcessError " << error.exitCode() << ": "
                   << error.what();
        return 1;
    } catch (std::exception& e) {
        LOG(ERROR) << "Unhandled exception thrown: " << e.what();
        return 1;
    }
}
