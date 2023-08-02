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
parser.add_argument('--path', default=None)
args, remaining = parser.parse_known_args()
BASE_DIR = args.base_dir
CCACHE = args.ccache
CLANG_TIDY = args.clang_tidy
CACHE_TIDY = args.linter_cache
sys.argv = sys.argv[0:1] + remaining

# explicitly set path when provided
if args.path:
    os.environ['PATH'] = args.path

# configure ccache and clang-tidy
os.environ['CCACHE'] = CCACHE
os.environ['CLANG_TIDY'] = CLANG_TIDY

# force enable debugging in ccache so we get better insights
DEBUGDIR = BASE_DIR / 'debug'
os.environ["CCACHE_DEBUG"] = "true"
os.environ["CCACHE_DIR"] = DEBUGDIR.as_posix()
os.environ["CCACHE_DEBUGDIR"] = DEBUGDIR.as_posix()

# force verbose logging for linter-cache
LINTER_CACHE_LOGFILE = DEBUGDIR / 'cache_tidy.log'
os.environ["LINTER_CACHE_DEBUG"] = "1"
os.environ["LINTER_CACHE_LOGFILE"] = LINTER_CACHE_LOGFILE.as_posix()


def _cleanup() -> None:
    '''Remove any intermediate files to ensure clean slate'''
    shutil.rmtree(DEBUGDIR, ignore_errors=True)
    DEBUGDIR.mkdir(exist_ok=True, parents=True)


def _configure_buildtree(src_dir: Path, build_dir: Path) -> None:
    '''Configure the buildtree used to test against'''
    subprocess.check_call(['cmake', '-S', src_dir.as_posix(), '-B', build_dir.as_posix(),
                          '-G', 'Ninja'])
    subprocess.check_call(
        ['cmake', '--build', build_dir.as_posix()])


def _prepare_buildtree(in_src_dir: Path, src_dir: Path, build_dir: Path) -> None:
    '''Prepare the buildtree used to test against'''
    shutil.rmtree(src_dir, ignore_errors=True)
    shutil.copytree(in_src_dir, src_dir, dirs_exist_ok=True)
    build_dir.mkdir(exist_ok=True, parents=True)
    _configure_buildtree(src_dir, build_dir)


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

    TEST_PROJ_DIR = Path(__file__).parent.resolve()
    SRC_DIR = BASE_DIR / 'src'
    BUILD_DIR = BASE_DIR / 'build'
    TESTED_FILE = SRC_DIR / 'hello_world.cpp'
    TESTED_CONFIG = SRC_DIR / '.clang-tidy'
    TESTED_PROJ = SRC_DIR / 'CMakeLists.txt'

    def _run(self, extra_env: dict = None, extra_args: list = None, check: bool = True):
        env = os.environ.copy()
        if extra_env:
            env.update(extra_env)
        args = [CACHE_TIDY,
                '-p', TestClangTidy.BUILD_DIR.as_posix(),
                '--quiet']
        if extra_args:
            args += extra_args
        return subprocess.run(args + [TestClangTidy.TESTED_FILE.as_posix()], env=env, stdout=subprocess.PIPE, stderr=subprocess.PIPE, encoding='utf8', check=check)

    def _fill_cache(self):
        '''Ensures we can expect a cache hit'''
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

    def test_source_modification(self):
        _cleanup()
        _prepare_buildtree(TestClangTidy.TEST_PROJ_DIR, TestClangTidy.SRC_DIR, TestClangTidy.BUILD_DIR)
        self._fill_cache()

        stats = CCacheStats()

        # editing the input should result in a cache miss
        contents = TestClangTidy.TESTED_FILE.read_text()
        edited = contents.replace('<replace to edit>', 'testing')
        stats.zero()
        TestClangTidy.TESTED_FILE.write_text(edited)
        proc = self._run()
        self.assertFalse(proc.stderr, f"Running with --quiet so nothing should end up in stderr: '{proc.stderr}'")
        self.assertFalse(proc.stdout, f"Running with --quiet so nothing should end up in stdout: '{proc.stdout}'")
        self.assertEqual(1, stats.cacheable, msg=stats.print())
        self.assertEqual(0, stats.cache_hits, msg=stats.print())

        # changing back to the original contents should be a hit again
        stats.zero()
        TestClangTidy.TESTED_FILE.write_text(contents)
        self._run()
        self.assertEqual(1, stats.cacheable, msg=stats.print())
        self.assertEqual(1, stats.cache_hits, msg=stats.print())

    def test_config_modification(self):
        _cleanup()
        _prepare_buildtree(TestClangTidy.TEST_PROJ_DIR, TestClangTidy.SRC_DIR, TestClangTidy.BUILD_DIR)
        self._fill_cache()

        stats = CCacheStats()

        # editing the configuration should result in a cache miss
        contents = TestClangTidy.TESTED_CONFIG.read_text()
        edited = contents.replace('llvm-header-guard', 'llvm-*')
        stats.zero()
        TestClangTidy.TESTED_CONFIG.write_text(edited)
        self._run()
        self.assertEqual(1, stats.cacheable, msg=stats.print())
        self.assertEqual(0, stats.cache_hits, msg=stats.print())

        # changing back to the original contents should be a hit again
        stats.zero()
        TestClangTidy.TESTED_CONFIG.write_text(contents)
        self._run()
        self.assertEqual(1, stats.cacheable, msg=stats.print())
        self.assertEqual(1, stats.cache_hits, msg=stats.print())

    def test_param_modification(self):
        _cleanup()
        _prepare_buildtree(TestClangTidy.TEST_PROJ_DIR, TestClangTidy.SRC_DIR, TestClangTidy.BUILD_DIR)
        self._fill_cache()

        stats = CCacheStats()

        # changing compiler flags should result in a cache miss
        contents = TestClangTidy.TESTED_PROJ.read_text()
        edited = contents.replace('# set(ADD_BOGUS_INCLUDE TRUE)', 'set(ADD_BOGUS_INCLUDE TRUE)')
        stats.zero()
        TestClangTidy.TESTED_PROJ.write_text(edited)
        _configure_buildtree(TestClangTidy.SRC_DIR, TestClangTidy.BUILD_DIR)
        self._run()
        self.assertEqual(1, stats.cacheable, msg=stats.print())
        self.assertEqual(0, stats.cache_hits, msg=stats.print())

        # changing back to the original contents should be a hit again
        stats.zero()
        TestClangTidy.TESTED_PROJ.write_text(contents)
        _configure_buildtree(TestClangTidy.SRC_DIR, TestClangTidy.BUILD_DIR)
        self._run()
        self.assertEqual(1, stats.cacheable, msg=stats.print())
        self.assertEqual(1, stats.cache_hits, msg=stats.print())

    def test_error_logging(self):
        _cleanup()
        _prepare_buildtree(TestClangTidy.TEST_PROJ_DIR, TestClangTidy.SRC_DIR, TestClangTidy.BUILD_DIR)
        self._fill_cache()

        stats = CCacheStats()

        # introducing an error should result in a cache miss and logged output
        contents = TestClangTidy.TESTED_FILE.read_text()
        edited = contents.replace('// <replace to edit>', 'int error = "string";')
        stats.zero()
        TestClangTidy.TESTED_FILE.write_text(edited)
        proc = self._run(check=False)
        self.assertNotEqual(0, proc.returncode)
        self.assertIn("error generated", proc.stderr, f"stderr: '{proc.stderr}'\nstdout: '{proc.stdout}'")
        self.assertIn("[clang-diagnostic-error]", proc.stdout, f"stderr: '{proc.stderr}'\nstdout: '{proc.stdout}'")
        self.assertEqual(1, stats.cacheable, msg=stats.print())
        self.assertEqual(0, stats.cache_hits, msg=stats.print())

    def test_with_extra_args(self):
        _cleanup()
        _prepare_buildtree(TestClangTidy.TEST_PROJ_DIR, TestClangTidy.SRC_DIR, TestClangTidy.BUILD_DIR)

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

    def test_with_output_file(self):
        _cleanup()
        _prepare_buildtree(TestClangTidy.TEST_PROJ_DIR, TestClangTidy.SRC_DIR, TestClangTidy.BUILD_DIR)
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
