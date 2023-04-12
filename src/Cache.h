/*
 * Cache.h
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

#ifndef CACHE_H_
#define CACHE_H_

#include <string>

#include "Linter.h"
#include "CommandlineArguments.h"

class Cache
{
public:
    Cache(const std::string& ccache, const Environment& env);

    std::string executable() const { return _ccache; }

    void execute(const CommandlineArguments& args,
                 const Linter& linter,
                 const std::string& objectfile,
                 const std::string& sourcefile) const;

private:
    std::string invoke(const StringList& args) const;

    std::string _ccache;
};

#endif // CACHE_H_
