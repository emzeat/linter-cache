#
# conanfile.py
#
# Copyright (c) 2022 - 2023 Marius Zwicker
# All rights reserved.
#
# Unauthorized copying of this file, via any medium is strictly prohibited.
# This file is a proprietary and confidential part of a software product
# by the authors or part of its connected software and documentation.
#
# ANY FILE BEARING THIS HEADER SHALL NOT BE RELEASED TO THE
# PUBLIC, BE USED, BE MODIFIED OR BE DISTRIBUTED IN ANY WAY
# WITHOUT WRITTEN PERMISSION OF THE AUTHORS.
#

import os

from conan import ConanFile
from conan.tools.cmake import CMake, CMakeDeps, CMakeToolchain
from conan.tools.files import copy


class LinterCacheConan(ConanFile):
    name = "linter-cache"
    license = "Apache 2.0"
    description = "Wrapper to invoke code linters through ccache to accelerate analysis as part of a regular compile job."
    url = "https://emzeat.de/linter-cache"

    settings = "os", "compiler", "build_type", "arch"

    options = {
        "build_tests": [True, False]
    }
    default_options = {
        "build_tests": True
    }

    def export_sources(self):
        self.copy("*", excludes=["build/*-*-*"])

    def build_requirements(self):
        self.tool_requires("ccache/4.6")
        self.tool_requires("clang-tools-extra/13.0.1@emzeat/external")
        if self.options.build_tests:
            self.test_requires("gtest/1.14.0")

    def generate(self):
        try:
            gtest = self.dependencies["gtest"]
            copy(self, "*.dll", src=gtest.cpp_info.bindirs[0], dst="../bin")
        except KeyError:
            pass

        deps = CMakeDeps(self)
        deps.build_context_activated = ["clang-tools-extra", "ccache"]
        deps.build_context_build_modules = ["clang-tools-extra", "ccache"]
        deps.generate()

        tc = CMakeToolchain(self, generator='Ninja')
        tc.cache_variables["MZ_DO_AUTO_FORMAT"] = False
        tc.cache_variables["MZ_DO_CPPLINT"] = False
        tc.cache_variables["MZ_DO_CPPLINT_DIFF"] = False
        tc.cache_variables['BUILD_LINTER_CACHE_TESTS'] = self.options.build_tests
        tc.generate()

    def _configure_cmake(self):
        cmake = CMake(self)
        cmake.configure(variables={'CONAN_EXPORTED': True})
        return cmake

    def build(self):
        cmake = self._configure_cmake()
        cmake.build()

    def package(self):
        cmake = self._configure_cmake()
        cmake.install()
        copy(self, "LICENSE", src=self.source_folder, dst="licenses")

    def package_info(self):
        self.cpp_info.bindirs = ['bin']
