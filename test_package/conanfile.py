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

from conans import ConanFile


class TestPackageConan(ConanFile):
    test_type = "explicit"

    def build_requirements(self):
        self.build_requires(self.tested_reference_str)

    def build(self):
        pass

    def test(self):
        self.run('linter-cache -h')
