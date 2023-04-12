/*
 * TemporaryFile.cpp
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
#if LINTER_CACHE_HAVE_GETPID
    #include <sys/types.h>
    #include <unistd.h>
#endif

#include "TemporaryFile.h"

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
        std::vector<char> buffer(2048, '\0');
        while (fgets(buffer.data(), static_cast<int>(buffer.size()), input)) {
            out.push_back(buffer.data());
            buffer[0] = '\0';
        }
        if (buffer[0] != '\0') {
            out.push_back(buffer.data());
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
        std::array<char, 512> buffer;
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
#endif
}

static int
processId()
{
#if LINTER_CACHE_HAVE_GETPID
    return getpid();
#else
    #error "No process id on this platform"
#endif
}

TemporaryFile::TemporaryFile()
  : NamedFile()
{
#if LINTER_CACHE_HAVE_MKTEMP
    // by making our PID part of the template there is only _very_
    // little chance of a collision even though we not use actually
    // safe methods like tmpfile(). The latter would make it really
    // hard to determine the underlying filename though
    static auto filenameTemplate =
      "/tmp/linter-cache-" + std::to_string(processId()) + "-XXXXXX";

    std::vector<char> buffer;
    buffer.resize(filenameTemplate.size() + 1);
    std::memcpy(
      buffer.data(), filenameTemplate.data(), filenameTemplate.size());
    _filename = mktemp(buffer.data());
#else
    #error "Cannot generate a temporary file name"
#endif
}

TemporaryFile::~TemporaryFile()
{
    if (!_filename.empty()) {
        unlink();
    }
}
