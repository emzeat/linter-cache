/*
 * Util.h
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

#ifndef UTIL_H_
#define UTIL_H_

#include <string>

class Util
{
public:
    // true when the given filepath points to a regular file
    static bool is_file(const std::string& filepath);

    // replaces all occurences of old_value in input with the new_value
    // and returns the resulting string
    static std::string replace_all(std::string input,
                                   const std::string& old_value,
                                   const std::string& new_value);

    // resolves the given path making it absolute without any symlinks
    static std::string resolve_path(const std::string& filepath);

    // searchs the parent directory of filepath and any parent directories
    // above for a config file with the given name and returns its path
    static std::string find_applicable_config(const std::string& conf_name,
                                              const std::string& filepath);

    // generates an annotation as created by the preprocessor when including
    // the given filepath
    static std::string preproc_file_header(const std::string& filepath);

private:
    Util() = delete;
};

#endif // UTIL_H_
