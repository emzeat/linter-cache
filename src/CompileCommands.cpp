/*
 * CompileCommands.cpp
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

#include <cassert>

#include "CompileCommands.h"
#include "TemporaryFile.h"
#include "Util.h"

CompileCommands::CompileCommands(const std::string& filepath)
  : _filepath(filepath)
{}

StringList
CompileCommands::linesForFile(const std::string& sourcefile) const
{
    assert(Util::is_file(_filepath));
#if _WIN32
    const auto sourcefile_backward = Util::replace_all(sourcefile, "/", "\\\\");
#endif

    StringList out;
    NamedFile input(_filepath);
    // as a minimal performance tuning just iterate
    // all lines of the file and spare the overhead
    // to do a full json parsing
    for (const auto& line : input.readLines()) {
        // match any lines with our filename, while
        // this might cause false positives it is good
        // enough and avoids more complicated matching logic
        if (line.find(sourcefile) != std::string::npos) {
            out.push_back(line);
        }
#if _WIN32
        // Compile db on Windows is using two backslashes
        if (line.find(sourcefile_backward) != std::string::npos) {
            out.push_back(line);
        }
#endif
    }
    return out;
}

StringList
CompileCommands::flagsForFile(const std::string& sourcefile,
                              bool skipCompiler) const
{
    StringList flags;

    auto lines = linesForFile(sourcefile);
    bool skip = skipCompiler;
    for (const auto& line : lines) {
        auto start = line.find("command\": \"");
        if (start != std::string::npos) {
            start += 11;
            auto end = line.find_first_of(" \"", start);
            while (end != std::string::npos) {
                const auto len = end - start;
                if (len > 0) {
                    const auto item = line.substr(start, len);
                    if (skip) {
                        // skip this item but parse the next
                        skip = false;
                    } else if ("-o" == item || "-c" == item) {
                        // skip this and the next which is the argument
                        skip = true;
                    } else {
                        flags.push_back(item);
                    }
                }
                start = end + 1;
                end = line.find_first_of(" \"", start);
            }
            break;
        }
    }
    return flags;
}
