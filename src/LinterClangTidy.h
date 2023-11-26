/*
 * LinterClangTidy.h
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

#ifndef LINTER_CLANG_TIDY_H_
#define LINTER_CLANG_TIDY_H_

#include "Linter.h"
#include "Subprocess.h"

class LinterClangTidy : public Linter
{
public:
    LinterClangTidy(const std::string& clangTidy, const Environment& env);

    std::string executable() const final { return _clangTidy; }

    void prepare(const std::string& sourceFile,
                 const CommandlineArguments& args,
                 SavedArguments& savedArgs,
                 Environment& env) final;

    void preprocess(const SavedArguments& savedArgs, std::string& output) final;

    void execute(const SavedArguments& savedArg, std::string& output) final;

private:
    std::string invoke(const StringList& args,
                       int flags = Process::Flags::NONE) const;

    std::string _clangTidy;
};

#endif // LINTER_CLANG_TIDY_H_
