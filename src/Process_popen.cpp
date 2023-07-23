/*
 * Process_popen.cpp
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

#include <array>
#include <iostream>
#include <cstring>

#include "Process.h"
#include "Logging.h"

#include "config.h"

#if LINTER_CACHE_HAVE_POPEN
    #include <cstdio>
#elif LINTER_CACHE_HAVE__POPEN
    #include <cstdio>

inline FILE*
popen(const char* command, const char* mode)
{
    return _popen(command, mode);
}

inline int
pclose(FILE* stream)
{
    return _pclose(stream);
}
#endif

void
Process::run()
{
    const auto cmd = _cmd.join(" ");
    _stdout.clear();
    _stderr.clear();

    if (_cmd.empty()) {
        LOG(ERROR) << "Empty command: '" << cmd << "'";
        throw ProcessError(cmd, -1);
    }

    auto* stdoutHandle = popen(cmd.c_str(), "r");
    if (nullptr == stdoutHandle) {
        throw ProcessError(cmd, -1);
    }

    std::array<char, 512> buffer;
    while (fgets(buffer.data(), buffer.size(), stdoutHandle)) {
        _stdout.append(buffer.data());
        if (0 != (_flags & Flags::FORWARD_OUTPUT)) {
            std::cout << buffer.data();
        }
        buffer[0] = '\0';
    }
    _stdout.append(buffer.data());
    _exitCode = pclose(stdoutHandle);
    if (0 != (_flags & Flags::FORWARD_OUTPUT)) {
        std::cout << std::flush;
    }
    if (0 != _exitCode) {
        throw ProcessError(cmd, _exitCode);
    }
}
