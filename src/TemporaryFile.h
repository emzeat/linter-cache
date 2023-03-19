/*
 * TemporaryFile.h
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

#ifndef TEMPORARY_FILE_H_
#define TEMPORARY_FILE_H_

#include <string>

class NamedFile
{
public:
    NamedFile(const std::string& filename = std::string());
    virtual ~NamedFile() = default;

    std::string readText() const;
    bool writeText(const std::string&);

    inline std::string filename() const { return _filename; }

    inline operator bool() const { return !_filename.empty(); }

protected:
    std::string _filename;
};

class TemporaryFile : public NamedFile
{
public:
    TemporaryFile();
    ~TemporaryFile();
};

#endif // TEMPORARY_FILE_H_
