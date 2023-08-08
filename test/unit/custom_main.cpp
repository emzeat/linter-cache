/*
 * custom_main.cpp
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

#include <iostream>
#include <cstring>
#include <chrono>
#include <thread>

#include "OutputGenerator.h"

int
main(int argc, char* argv[])
{
    for (int i = 1; i + 1 < argc; ++i) {
        if (0 == std::strcmp(argv[i], "--stderr")) {
            std::cerr << argv[++i] << std::flush;
        }
        if (0 == std::strcmp(argv[i], "--generate-stderr")) {
            std::cerr << generateStringWithLength(atoi(argv[++i]))
                      << std::flush;
        }
        if (0 == std::strcmp(argv[i], "--stdout")) {
            std::cout << argv[++i] << std::flush;
        }
        if (0 == std::strcmp(argv[i], "--generate-stdout")) {
            std::cout << generateStringWithLength(atoi(argv[++i]))
                      << std::flush;
        }
        if (0 == std::strcmp(argv[i], "--sleep")) {
            std::this_thread::sleep_for(std::chrono::seconds(atoi(argv[++i])));
        }
        if (0 == std::strcmp(argv[i], "--exit")) {
            return atoi(argv[++i]);
        }
    }

    return 0;
}
