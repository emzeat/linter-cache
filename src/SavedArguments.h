/*
 * SavedArguments.h
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

#ifndef SAVED_ARGUMENTS_H_
#define SAVED_ARGUMENTS_H_

#include <memory>
#include <string>
#include <map>

#include "TemporaryFile.h"
#include "Environment.h"

class SavedArguments
{
public:
    static const char* kDefaultEnvVariable;

    SavedArguments();
    ~SavedArguments();

    void save(Environment& env, const char* envVariable = kDefaultEnvVariable);
    void load(Environment& env, const char* envVariable = kDefaultEnvVariable);

    void set(const char* key, const std::string& value);
    std::string get(const char* key,
                    const std::string& defaultValue = std::string()) const;

    inline operator bool() const { return !_arguments.empty(); }

private:
    std::unique_ptr<NamedFile> _file;
    std::map<std::string, std::string> _arguments;
};

#endif // SAVED_ARGUMENTS_H_
