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
    StringList(const std::string& singleElement)
      : Base({ singleElement })
    {}
    StringList(std::initializer_list<std::string> elements)
      : Base(elements)
    {}
    StringList(char const* const* elements, size_t count)
      : Base(elements, elements + count)
    {}

    std::string join(const std::string& separator) const
    {
        std::string ret;
        size_t i = 0;
        for (const auto& string : *this) {
            ret += string;
            if (++i < size()) {
                ret += separator;
            }
        }
        return ret;
    }
};

#endif // STRING_LIST_H_
