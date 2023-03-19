/*
 * Environment.h
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

#ifndef ENVIRONMENT_H_
#define ENVIRONMENT_H_

#include <string>
#include <map>
#include <set>

class Environment
{
public:
    inline ~Environment() { reset(); }
    void reset();

    static std::string get(const char* key,
                           const std::string& defaultValue = std::string());
    static int get(const char* key, int defaultValue);
    static double get(const char* key, double defaultValue);

    void unset(const char* key);
    void set(const char* key, const std::string& value);
    inline void set(const char* key, int value)
    {
        set(key, std::to_string(value));
    }
    inline void set(const char* key, double value)
    {
        set(key, std::to_string(value));
    }

private:
    std::set<std::string> _variables;
    std::map<std::string, std::string> _originals;
};

#endif // ENVIRONMENT_H_
