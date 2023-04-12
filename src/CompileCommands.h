/*
 * CompileCommands.h
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

#ifndef COMPILE_COMMANDS_H_
#define COMPILE_COMMANDS_H_

#include <string>

#include "StringList.h"

class CompileCommands
{
public:
    CompileCommands(const std::string& filepath);

    StringList linesForFile(const std::string& sourcefile) const;

private:
    std::string _filepath;
};

#endif // COMPILE_COMMANDS_H_
