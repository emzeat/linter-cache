/*
 * NamedFile.cpp
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

#include "config.h"

#include <cstdio>
#include <cstring>
#include <array>
#include <vector>
#if LINTER_CACHE_HAVE_UNLINK
    #include <sys/types.h>
    #include <unistd.h>
#endif
#if LINTER_CACHE_HAVE_DELETE_FILE
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
#endif

#include "NamedFile.h"

NamedFile::NamedFile(const std::string& filename)
  : _filename(filename)
{}

StringList
NamedFile::readLines() const
{
    StringList out;
    if (_filename.empty()) {
        return out;
    }

    auto* input = fopen(_filename.c_str(), "r");
    if (input) {
        std::string current;
        std::array<char, 2048> buffer;
        while (fgets(buffer.data(), static_cast<int>(buffer.size()), input)) {
            current.append(buffer.data());
            if (!current.empty() && current.back() == '\n') {
                current.pop_back(); // drop newline
                out.push_back(current);
                current.clear();
            }
            buffer[0] = '\0';
        }
        if (buffer[0] != '\0') {
            current.append(buffer.data());
        }
        if (!current.empty()) {
            out.push_back(current);
        }
        fclose(input);
    }
    return out;
}

std::string
NamedFile::readText() const
{
    std::string text;
    if (_filename.empty()) {
        return text;
    }

    auto* input = fopen(_filename.c_str(), "r");
    if (input) {
        std::array<char, 2048> buffer;
        buffer[0] = '\0';
        while (fgets(buffer.data(), buffer.size(), input)) {
            text.append(buffer.data());
            buffer[0] = '\0';
        }
        text.append(buffer.data());
        fclose(input);
    }
    return text;
}

bool
NamedFile::writeText(const std::string& text)
{
    if (_filename.empty()) {
        return false;
    }

    auto* output = fopen(_filename.c_str(), "w");
    if (output) {
        fputs(text.c_str(), output);
        fclose(output);
        return true;
    }

    return false;
}

void
NamedFile::unlink()
{
#if LINTER_CACHE_HAVE_UNLINK
    ::unlink(_filename.c_str());
#elif LINTER_CACHE_HAVE_DELETE_FILE
    DeleteFileA(_filename.c_str());
#else
    #error "Cannot unlink files"
#endif
}
