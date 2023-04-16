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
    if (LINTER_CACHE_HAVE_GET_FILE_ATTRIBUTES != attr) {
        return attr & FILE_ATTRIBUTE_NORMAL;
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