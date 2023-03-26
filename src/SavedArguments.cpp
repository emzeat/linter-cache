/*
 * SavedArguments.cpp
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

#include "SavedArguments.h"
#include "Logging.h"

const char* SavedArguments::kDefaultEnvVariable = "LINTER_CACHE_ARGS";

SavedArguments::SavedArguments() = default;
SavedArguments::~SavedArguments() = default;

static std::string
escapeString(std::string string)
{
    for (size_t i = 0; i < string.size(); ++i) {
        if ('\n' == string[i]) {
            string.replace(i, 1, "\\n");
        }
    }
    return string;
}

static std::string
unescapeString(std::string string)
{
    for (size_t i = 0; i < string.size(); ++i) {
        if ("\\n" == string.substr(i, 2)) {
            string.replace(i, 2, "\n");
        }
    }
    return string;
}

void
SavedArguments::save(Environment& env, const char* envVariable)
{
    if (_file) {
        std::string saved;
        for (const auto& arg : _arguments) {
            saved +=
              escapeString(arg.first) + "\n" + escapeString(arg.second) + "\n";
        }
        if (_file->writeText(saved)) {
            env.set(envVariable, _file->filename());
        }
    }
}

void
SavedArguments::load(Environment& env, const char* envVariable)
{
    auto filename = env.get(envVariable);
    if (filename.empty()) {
        LOG(ERROR) << "No environment in env: " << envVariable;
        return;
    }

    _file = std::make_unique<NamedFile>(filename);
    auto saved = _file->readText();
    while (!saved.empty()) {
        auto keyDelimiter = saved.find_first_of('\n');
        if (std::string::npos == keyDelimiter) {
            LOG(ERROR) << "Failed to find key delimiter in:\n" << saved;
            break;
        }
        auto valueDelimiter = saved.find_first_of('\n', keyDelimiter + 1);
        if (std::string::npos == valueDelimiter) {
            LOG(ERROR) << "Failed to find value delimiter in:\n" << saved;
            break;
        }

        auto key = saved.substr(0, keyDelimiter);
        key = unescapeString(key);
        auto value =
          saved.substr(keyDelimiter + 1, valueDelimiter - keyDelimiter - 1);
        value = unescapeString(value);
        _arguments[key] = value;

        saved = saved.substr(valueDelimiter + 1);
    }
}

void
SavedArguments::set(const char* key, const std::string& value)
{
    if (!_file) {
        _file = std::make_unique<TemporaryFile>();
    }

    _arguments[key] = value;
}

std::string
SavedArguments::get(const char* key, const std::string& defaultValue) const
{
    auto it = _arguments.find(key);
    if (it != _arguments.end()) {
        return it->second;
    }
    return defaultValue;
}
