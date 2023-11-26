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
#include <exception>
#if LINTER_CACHE_HAVE_GETPID
    #include <sys/types.h>
    #include <unistd.h>
#endif
#if LINTER_CACHE_HAVE_GET_CURRENT_PROCESS_ID ||                                \
  LINTER_CACHE_HAVE_GET_TEMP_FILE_NAME
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
#endif

#include "TemporaryFile.h"

TemporaryFile::TemporaryFile()
  : NamedFile()
{
    // by making our PID part of the template there is only _very_
    // little chance of a collision even though we not use actually
    // safe methods like tmpfile(). The latter would make it really
    // hard to determine the underlying filename though
#if LINTER_CACHE_HAVE_MKSTEMP && LINTER_CACHE_HAVE_GETPID &&                   \
  LINTER_CACHE_HAVE_CLOSE
    _filename = "/tmp/linter-cache-" + std::to_string(getpid()) + "-XXXXXX";
    auto fd = mkstemp(_filename.data());
    if (fd >= 0) {
        close(fd);
    } else {
        throw std::runtime_error(
          std::string("Failed to create temporary file: ") + strerror(errno));
    }
#elif LINTER_CACHE_HAVE_GET_TEMP_FILE_NAME &&                                  \
  LINTER_CACHE_HAVE_GET_CURRENT_PROCESS_ID
    std::vector<char> tempPathBuffer(1);
    auto pathLen = GetTempPathA(tempPathBuffer.size(), tempPathBuffer.data());
    if (pathLen > tempPathBuffer.size()) {
        tempPathBuffer.resize(pathLen + 1);
        GetTempPathA(tempPathBuffer.size(), tempPathBuffer.data());
    }
    std::vector<char> buffer(MAX_PATH);
    GetTempFileNameA(
      tempPathBuffer.data(), "lc-", GetCurrentProcessId(), buffer.data());
    _filename = buffer.data();
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
