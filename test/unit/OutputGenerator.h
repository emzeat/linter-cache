/*
 * OutputGenerator.h
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

#ifndef OUTPUT_GENERATOR_H_
#define OUTPUT_GENERATOR_H_

#include <string>

static std::string generateStringWithLength( std::size_t length ) {
    std::string output;
    output.reserve(length);
    for (size_t i = 0; i < length; ++i) {
        output.push_back(static_cast<char>('a' + (i % 26)));
    }
    return output;
}

#endif // OUTPUT_GENERATOR_H_
