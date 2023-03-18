#
# CMakeLists.txt
#
# Copyright (c) 2011 - 2023 Marius Zwicker
# All rights reserved.
#
# SPDX-License-Identifier: Apache-2.0
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

cmake_minimum_required(VERSION 3.1)
project(linter-cache CXX C)

# build scripts
if(NOT HAS_MZ_GLOBAL)
    include(build/global.cmake)
    include(build/macros.cmake)
    include(build/conan.cmake)
endif()

find_program(PYTHON3 python3 python REQUIRED)
find_program(CCACHE ccache REQUIRED)
find_program(CLANG_TIDY clang-tidy REQUIRED)

enable_testing()
list(APPEND INTEGRATION_TESTS
	TestClangTidy.test_basic_run
	TestClangTidy.test_with_extra_args
	TestClangTidy.test_with_output
)
foreach(_test IN LISTS INTEGRATION_TESTS)
	add_test(
	    NAME ${_test}
	    COMMAND ${PYTHON3} test.py ${_test}
	        --base-dir ${CMAKE_BINARY_DIR}/integration
	        --ccache ${CCACHE}
	        --clang-tidy ${CLANG_TIDY}
	    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
	)
endforeach()
