/*
 * Process.cpp
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

#include "Process.h"

#include "config.h"

#if LINTER_CACHE_HAVE_POPEN
    #include <cstdio>
#endif

Process::Process(const StringList& cmd)
  : _cmd(cmd.join(" "))
  , _exitCode(-1)
{}

Process::Process(const char* cmd)
  : _cmd(cmd)
  , _exitCode(-1)
{}

void
Process::run()
{
    _output.clear();

#if LINTER_CACHE_HAVE_POPEN

    // FIXME(zwicker): popen() is easy but also lazy as it requires
    //                 and additional spawn of a shell
    auto* stdout = popen(_cmd.c_str(), "r");
    if (nullptr == stdout) {
        throw ProcessError(_cmd, -1);
    }

    std::array<char, 512> buffer;
    while (fgets(buffer.data(), buffer.size(), stdout)) {
        _output.append(buffer.data());
        buffer[0] = '\0';
    }
    _output.append(buffer.data());
    _exitCode = pclose(stdout);
    if (0 != _exitCode) {
        throw ProcessError(_cmd, _exitCode);
    }
#else
    #error "No process implementation on this platform
#endif
}
