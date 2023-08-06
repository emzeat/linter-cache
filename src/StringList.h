/*
 * StringList.h
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

#ifndef STRING_LIST_H_
#define STRING_LIST_H_

#include <string>
#include <vector>

class StringList : public std::vector<std::string>
{
public:
    using Base = std::vector<std::string>;

    StringList() = default;
    explicit StringList(const std::string& singleElement);
    StringList(std::initializer_list<std::string> elements);
    StringList(char const* const* elements, size_t count);

    std::string join(char separator) const;
    std::string join(const std::string& separator) const;
    static StringList split(const std::string& input, char delimiter);
    static StringList split(const std::string& input,
                            const std::string& delimiter);

    friend StringList operator+(const StringList& lhs, const std::string& rhs);
    friend StringList operator+(const std::string& lhs, const StringList& rhs);
    friend StringList operator+(const StringList& lhs, const StringList& rhs);

    StringList& operator+=(const std::string& other);
    StringList& operator+=(const StringList& other);

    friend std::ostream& operator<<(std::ostream& stream,
                                    const StringList& list);
};

#endif // STRING_LIST_H_
