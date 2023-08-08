/*
 * Util.cpp
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

#include <vector>
#include <climits>
#include <cstdlib>

#include "config.h"

#include "Util.h"

#if LINTER_CACHE_HAVE_GET_FILE_ATTRIBUTES
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
#elif LINTER_CACHE_HAVE_STAT
    #include <sys/types.h>
    #include <sys/stat.h>
    #include <unistd.h>
#endif

bool
Util::is_file(const std::string& filepath)
{
#if LINTER_CACHE_HAVE_GET_FILE_ATTRIBUTES
    auto attr = GetFileAttributesA(filepath.c_str());
    switch (attr) {
        case FILE_ATTRIBUTE_ARCHIVE:
        case FILE_ATTRIBUTE_NORMAL:
        case FILE_ATTRIBUTE_TEMPORARY:
            return true;
    }
    return false;
#elif LINTER_CACHE_HAVE_STAT
    struct stat result;
    if (0 == stat(filepath.c_str(), &result)) {
        return S_ISREG(result.st_mode);
    }
    return false;
#else
    #error "Cannot stat on this platform"
#endif
}

std::string
Util::resolve_path(const std::string& filepath)
{
#if LINTER_CACHE_HAVE_GET_FULL_PATHNAME
    static constexpr auto kPathMax = 512;
    std::vector<char> buffer(kPathMax);
    auto len =
      GetFullPathNameA(filepath.c_str(), buffer.size(), buffer.data(), nullptr);
    if (len > 0 && len <= buffer.size()) {
        // make backward into forward slashes to maximize
        // compatibility with other places in the code
        for (size_t i = 0; i < len; ++i) {
            auto& character = buffer[i];
            if ('\\' == character) {
                character = '/';
            }
        }
        return std::string(buffer.data(), len);
    }
    return std::string();
#else
    std::vector<char> buffer;
    buffer.resize(PATH_MAX);
    auto* resolved = realpath(filepath.c_str(), buffer.data());
    if (resolved) {
        return std::string(resolved);
    }
    return std::string();
#endif
}

std::string
Util::replace_all(std::string input,
                  const std::string& old_value,
                  const std::string& new_value)
{
    for (size_t pos = input.find(old_value, 0); pos != std::string::npos;
         pos = input.find(old_value, pos)) {
        input.replace(pos, old_value.length(), new_value);
        pos += new_value.length();
    }
    return input;
}

std::string
Util::find_applicable_config(const std::string& conf_name,
                             const std::string& filepath)
{
    const auto input = resolve_path(filepath);
    auto end = input.find_last_of("/\\");
    while (end > 0 && end != std::string::npos) {
        auto candidate = input.substr(0, end) + "/" + conf_name;
        if (is_file(candidate)) {
            return candidate;
        }
        end = input.find_last_of("/\\", end - 1);
    }
    return std::string();
}

std::string
Util::preproc_file_header(const std::string& filepath)
{
    return "\n# 1 \"" + filepath + "\" 1\n";
}
