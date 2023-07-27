/*
 * StringList.cpp
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

#include <iostream>

#include "StringList.h"

StringList::StringList(const std::string& singleElement)
  : Base({ singleElement })
{}
StringList::StringList(std::initializer_list<std::string> elements)
  : Base(elements)
{}
StringList::StringList(char const* const* elements, size_t count)
  : Base(elements, elements + count)
{}

std::string
StringList::join(const std::string& separator) const
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

StringList
StringList::split(const std::string& input, char delimiter)
{
    StringList result;
    size_t start = 0;
    auto end = input.find_first_of(delimiter);
    while (end != std::string::npos) {
        result.push_back(input.substr(start, end - start));
        start = end + 1;
        end = input.find_first_of(delimiter, start);
    }
    if (start < end) {
        result.push_back(input.substr(start));
    }
    return result;
}

StringList
operator+(const StringList& lhs, const std::string& rhs)
{
    StringList copy(lhs);
    copy.push_back(rhs);
    return copy;
}

StringList
operator+(const std::string& lhs, const StringList& rhs)
{
    StringList copy(lhs);
    copy.insert(copy.end(), rhs.begin(), rhs.end());
    return copy;
}

StringList
operator+(const StringList& lhs, const StringList& rhs)
{
    StringList copy(lhs);
    copy.insert(copy.end(), rhs.begin(), rhs.end());
    return copy;
}

StringList&
StringList::operator+=(const std::string& other)
{
    push_back(other);
    return *this;
}

StringList&
StringList::operator+=(const StringList& other)
{
    insert(end(), other.begin(), other.end());
    return *this;
}

std::ostream&
operator<<(std::ostream& stream, const StringList& list)
{
    return stream << "[" << list.join(", ") << "]";
}
