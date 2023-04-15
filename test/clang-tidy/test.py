#
# test.py
#
# Copyright (c) 2023 Marius Zwicker
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

import argparse
import os
import re
import shutil
import subprocess
import sys
import unittest
from pathlib import Path

BASE_DIR = (Path(__file__).parent / 'test').resolve()

CLANG_TIDY = shutil.which('clang-tidy')
CCACHE = shutil.which('ccache')
CACHE_TIDY = None

# CLI overrides
parser = argparse.ArgumentParser(prog='integration_test')
parser.add_argument('--base-dir', default=BASE_DIR, type=Path)
parser.add_argument('--ccache', default=CCACHE)
parser.add_argument('--clang-tidy', default=CLANG_TIDY)
parser.add_argument('--linter-cache', required=True, type=Path)
args, remaining = parser.parse_known_args()
BASE_DIR = args.base_dir
CCACHE = args.ccache
CLANG_TIDY = args.clang_tidy
CACHE_TIDY = args.linter_cache
sys.argv = sys.argv[0:1] + remaining

# configure ccache and clang-tidy
os.environ['CCACHE'] = CCACHE
os.environ['CLANG_TIDY'] = CLANG_TIDY

# force enable debugging in ccache so we get better insights
DEBUGDIR = BASE_DIR / 'debug'
os.environ["CCACHE_DEBUG"] = "true"
os.environ["CCACHE_DIR"] = DEBUGDIR.as_posix()
os.environ["CCACHE_DEBUGDIR"] = DEBUGDIR.as_posix()

# force verbose logging for cache-tidy
CACHE_TIDY_LOGFILE = DEBUGDIR / 'cache_tidy.log'
os.environ["CACHE_TIDY_VERBOSE"] = "1"
os.environ["CACHE_TIDY_LOGFILE"] = CACHE_TIDY_LOGFILE.as_posix()


def _cleanup() -> None:
    '''Remove any intermediate files to ensure clean slate'''
    shutil.rmtree(DEBUGDIR, ignore_errors=True)
    DEBUGDIR.mkdir(exist_ok=True, parents=True)


def _prepare_buildtree(src_dir: Path, build_dir: Path) -> None:
    '''Prepare the buildtree used to test against'''
    build_dir.mkdir(exist_ok=True, parents=True)
    subprocess.check_call(['cmake', '-S', src_dir.as_posix(), '-B', build_dir.as_posix(),
                          '-G', 'Ninja'], stdout=subprocess.DEVNULL)
    subprocess.check_call(
        ['cmake', '--build', build_dir.as_posix()], stdout=subprocess.DEVNULL)


class CCacheStats:

    def zero(self):
        subprocess.check_call([CCACHE, '--zero-stats'],
                              stdout=subprocess.DEVNULL)

    def _stats(self):
        stats = subprocess.check_output(
            [CCACHE, '--show-stats'], encoding='utf8')
        return stats.split('\n')

    def _version(self):
        return subprocess.check_output([CCACHE, '--version'], encoding='utf8').split('\n')[0].strip()

    def print(self):
        print(self._version())
        print('\n'.join(self._stats()))

    @property
    def cache_hits(self) -> int:
        for line in self._stats():
            match = re.search(r'Hits: +([0-9]+) /', line)
            if match:
                return int(match[1])
        return -1

    @property
    def cacheable(self) -> int:
        hits = None
        misses = None
        uncacheable = 0
        for line in self._stats():
            match = re.search(r'Cacheable calls: +([0-9]+) /', line)
            if match:
                return int(match[1])
            # more recent versions of ccache do not report cacheable calls anymore
            # calculate by adding hits, misses and uncacheables
            match = re.search(r'Hits: +([0-9]+) /', line)
            if match and hits is None:
                hits = int(match[1])
            match = re.search(r'Misses: +([0-9]+)', line)
            if match and misses is None:
                misses = int(match[1])
            match = re.search(r'Uncacheable: +([0-9]+)', line)
            if match:
                uncacheable = int(match[1])
        if hits is None or misses is None:
            return -1
        return hits + misses + uncacheable


class TestClangTidy(unittest.TestCase):

    SRC_DIR = Path(__file__).parent.resolve()
    BUILD_DIR = BASE_DIR / 'build'
    TESTED_FILE = SRC_DIR / 'hello_world.cpp'

    def _run(self, extra_env: dict = None, extra_args: list = None):
        env = os.environ.copy()
        if extra_env:
            env.update(extra_env)
        args = [CACHE_TIDY,
                '-p', TestClangTidy.BUILD_DIR.as_posix(),
                '--quiet']
        if extra_args:
            args += extra_args
        subprocess.check_call(args + [TestClangTidy.TESTED_FILE.as_posix()], env=env)

    def test_basic_run(self):
        _cleanup()
        _prepare_buildtree(TestClangTidy.SRC_DIR, TestClangTidy.BUILD_DIR)

        stats = CCacheStats()

        # first run should be cacheable but not in the cache yet
        stats.zero()
        self._run()
        self.assertEqual(1, stats.cacheable, msg=stats.print())
        self.assertEqual(0, stats.cache_hits, msg=stats.print())

        # second run should be served from the cache
        stats.zero()
        self._run()
        self.assertEqual(1, stats.cacheable, msg=stats.print())
        self.assertEqual(1, stats.cache_hits, msg=stats.print())

        # editing the input should result in a cache miss
        contents = TestClangTidy.TESTED_FILE.read_text()
        edited = contents.replace('<replace to edit>', 'testing')
        stats.zero()
        TestClangTidy.TESTED_FILE.write_text(edited)
        self._run()
        self.assertEqual(1, stats.cacheable, msg=stats.print())
        self.assertEqual(0, stats.cache_hits, msg=stats.print())

        # changing back to the original contents should be a hit again
        stats.zero()
        TestClangTidy.TESTED_FILE.write_text(contents)
        self._run()
        self.assertEqual(1, stats.cacheable, msg=stats.print())
        self.assertEqual(1, stats.cache_hits, msg=stats.print())

    def test_with_extra_args(self):
        _cleanup()
        _prepare_buildtree(TestClangTidy.SRC_DIR, TestClangTidy.BUILD_DIR)

        stats = CCacheStats()
        stats.zero()

        # first run should be cacheable but not in the cache yet
        self._run(extra_args=['--extra-arg=-DLINTING=1'])
        self.assertEqual(1, stats.cacheable, msg=stats.print())
        self.assertEqual(0, stats.cache_hits, msg=stats.print())

        # second run should be served from the cache
        self._run(extra_args=['--extra-arg=-DLINTING=1'])
        self.assertEqual(2, stats.cacheable, msg=stats.print())
        self.assertEqual(1, stats.cache_hits, msg=stats.print())

    def test_with_output(self):
        _cleanup()
        _prepare_buildtree(TestClangTidy.SRC_DIR, TestClangTidy.BUILD_DIR)
        output = TestClangTidy.BUILD_DIR / 'clang-tidy.stamp'

        stats = CCacheStats()
        stats.zero()

        # first run should be cacheable but not in the cache yet
        self._run(extra_args=[f'--linter-cache-o={output.as_posix()}'])
        self.assertEqual(1, stats.cacheable, msg=stats.print())
        self.assertEqual(0, stats.cache_hits, msg=stats.print())
        self.assertTrue(output.exists())
        output.unlink()

        # second run should be served from the cache
        self._run(extra_args=[f'--linter-cache-o={output.as_posix()}'])
        self.assertEqual(2, stats.cacheable, msg=stats.print())
        self.assertEqual(1, stats.cache_hits, msg=stats.print())
        self.assertTrue(output.exists())


if __name__ == '__main__':
    unittest.main()
