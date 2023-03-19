/*
 * Environment.cpp
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

#include "Environment.h"

#include "config.h"

#include <cstdlib>

void
Environment::reset()
{
    for (const auto& var : _variables) {
        auto original = _originals.find(var);
        if (original == _originals.end()) {
            unsetenv(var.c_str());
        } else {
            setenv(var.c_str(), original->second.c_str(), /* overwrite */ 1);
        }
    }

    _originals.clear();
    _variables.clear();
}

std::string
Environment::get(const char* key, const std::string& defaultValue)
{
    auto* value = getenv(key);
    if (value) {
        return std::string(value);
    }
    return defaultValue;
}

int
Environment::get(const char* key, int defaultValue)
{
    auto value = get(key, std::to_string(defaultValue));
    return std::atoi(value.c_str());
}

double
Environment::get(const char* key, double defaultValue)
{
    auto value = get(key, std::to_string(defaultValue));
    return std::atof(value.c_str());
}

void
Environment::unset(const char* key)
{
    set(key, std::string());
    unsetenv(key);
}

void
Environment::set(const char* key, const std::string& value)
{
    auto it = _variables.insert(key);
    if (it.second) {
        // value had not been set here before, remember original value (if any)
        auto* original = getenv(key);
        if (original != nullptr) {
            _originals[key] = original;
        }
    }

    setenv(key, value.c_str(), /* overwrite */ 1);
}
