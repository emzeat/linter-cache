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
#include <vector>
#include <stdexcept>

#if LINTER_CACHE_HAVE_SETENV && LINTER_CACHE_HAVE_GETENV &&                    \
  LINTER_CACHE_HAVE_UNSETENV
namespace platform {
static void
setenv(const char* name, const std::string& value)
{
    ::setenv(name, value.c_str(), /* overwrite */ 1);
}

static void
unsetenv(const char* name)
{
    ::unsetenv(name);
}

static std::string
getenv(const char* name)
{
    auto* value = ::getenv(name);
    if (value) {
        return std::string(value);
    }
    throw std::out_of_range(name);
}
}
#elif LINTER_CACHE_HAVE_SET_ENVIRONMENT_VARIABLE &&                            \
  LINTER_CACHE_HAVE_GET_ENVIRONMENT_VARIABLE

    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
    #include <array>

namespace platform {
static void
setenv(const char* name, const std::string& value)
{
    SetEnvironmentVariableA(name, value.c_str());
}

static void
unsetenv(const char* name)
{
    SetEnvironmentVariableA(name, nullptr);
}

static std::string
getenv(const char* name)
{
    std::vector<char> buffer(1024);
    auto stored = GetEnvironmentVariableA(name, buffer.data(), buffer.size());
    if (stored > buffer.size()) {
        buffer.resize(stored + 1);
        stored = GetEnvironmentVariableA(name, buffer.data(), buffer.size());
    }
    if (0 == stored) {
        throw std::out_of_range(name);
    }
    return std::string(buffer.data(), stored);
}
}
#else
    #error "Cannot interact with the env on this platform"
#endif

void
Environment::reset()
{
    for (const auto& var : _variables) {
        auto original = _originals.find(var);
        if (original == _originals.end()) {
            platform::unsetenv(var.c_str());
        } else {
            platform::setenv(var.c_str(), original->second);
        }
    }

    _originals.clear();
    _variables.clear();
}

std::string
Environment::get(const char* key, const std::string& defaultValue)
{
    try {
        return platform::getenv(key);
    } catch (std::out_of_range&) {
        return defaultValue;
    }
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
    platform::unsetenv(key);
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

    platform::setenv(key, value);
}
