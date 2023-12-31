/*
 * Linter.h
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

#ifndef LINTER_H_
#define LINTER_H_

#include "SavedArguments.h"
#include "CommandlineArguments.h"
#include "Environment.h"
#include "StringList.h"

class Linter
{
public:
    virtual ~Linter() = default;

    virtual std::string executable() const = 0;

    virtual void prepare(const std::string& sourceFile,
                         const CommandlineArguments& args,
                         SavedArguments& savedArgs,
                         Environment& env) = 0;

    virtual void preprocess(const SavedArguments& savedArgs,
                            std::string& output) = 0;

    virtual void execute(const SavedArguments& savedArgs,
                         std::string& output) = 0;
};

#endif // LINTER_H_
