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

from conans import ConanFile, CMake


class LinterCacheConan(ConanFile):
    name = "linter-cache"
    license = "Apache 2.0"
    description = "Wrapper to invoke code linters through ccache to accelerate analysis as part of a regular compile job."
    url = "https://emzeat.de/linter-cache"

    generators = "cmake_find_package", "json"

    settings = "os", "compiler", "build_type", "arch"

    def build_requirements(self):
        self.tool_requires("clang-tools-extra/13.0.1@emzeat/external")

    def _configure_cmake(self):
        cmake = CMake(self)
        cmake.configure(source_folder=self.source_folder,
                        build_folder=self.build_folder)
        return cmake

    def build(self):
        cmake = self._configure_cmake()
        cmake.build()

    def package(self):
        cmake = self._configure_cmake()
        cmake.install()
        self.copy("LICENSE", src=self.source_folder, dst="licenses")
