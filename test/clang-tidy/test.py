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
        output = [self._version()] + self._stats()
        return '\n'.join(output)

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

    def _configure_buildtree(self, src_dir: Path, build_dir: Path) -> None:
        '''Configure the buildtree used to test against'''
        subprocess.check_call(['cmake', '-S', src_dir.as_posix(), '-B', build_dir.as_posix(),
                               '-G', 'Ninja'])
        subprocess.check_call(
            ['cmake', '--build', build_dir.as_posix()])

    def _prepare_buildtree(self, in_src_dir: Path = None, src_dir: Path = None, build_dir: Path = None) -> None:
        '''Prepare the buildtree used to test against'''
        if in_src_dir is None:
            in_src_dir = TestClangTidy.TEST_PROJ_DIR
        if src_dir is None:
            src_dir = TestClangTidy.SRC_DIR
        if build_dir is None:
            build_dir = TestClangTidy.BUILD_DIR
        shutil.rmtree(src_dir, ignore_errors=True)
        shutil.copytree(in_src_dir, src_dir, dirs_exist_ok=True)
        build_dir.mkdir(exist_ok=True, parents=True)
        self._configure_buildtree(src_dir, build_dir)
        self.SRC_DIR = src_dir
        self.BUILD_DIR = build_dir
        self.TESTED_FILE = src_dir / 'hello_world.cpp'
        self.TESTED_CONFIG = src_dir / '.clang-tidy'
        self.TESTED_PROJ = src_dir / 'CMakeLists.txt'

    def _run(self, extra_env: dict = None, extra_args: list = None, check: bool = True):
        env = os.environ.copy()
        if extra_env:
            env.update(extra_env)
        args = [CACHE_TIDY,
                '-p', self.BUILD_DIR.as_posix(),
                '--quiet']
        if extra_args:
            args += extra_args

        def dump_output(proc):
            sys.stdout.write(proc.stdout)
            sys.stdout.flush()
            sys.stderr.write(proc.stderr)
            sys.stderr.flush()
        try:
            proc = subprocess.run(args + [self.TESTED_FILE.as_posix()], env=env, cwd=self.BUILD_DIR,
                                  stdout=subprocess.PIPE, stderr=subprocess.PIPE, encoding='utf8', check=check)
        except subprocess.CalledProcessError as error:
            dump_output(error)
            raise
        if 0 != proc.returncode:
            dump_output(proc)
        return proc

    def _fill_cache(self):
        '''Ensures we can expect a cache hit'''
        stats = CCacheStats()
        # first run should be cacheable but not in the cache yet
        print("Populating cache...")
        stats.zero()
        self._run()
        self.assertEqual(1, stats.cacheable, msg=stats.print())
        self.assertEqual(0, stats.cache_hits, msg=stats.print())
        # second run should be served from the cache
        print("Verifying cache...")
        stats.zero()
        self._run()
        self.assertEqual(1, stats.cacheable, msg=stats.print())
        self.assertEqual(1, stats.cache_hits, msg=stats.print())

    def test_source_modification(self):
        _cleanup()
        self._prepare_buildtree()
        self._fill_cache()

        stats = CCacheStats()

        # editing the input should result in a cache miss
        contents = self.TESTED_FILE.read_text()
        edited = contents.replace('<replace to edit>', 'testing')
        stats.zero()
        self.TESTED_FILE.write_text(edited)
        print("Verifying after modification...")
        proc = self._run()
        self.assertFalse(proc.stderr, f"Running with --quiet so nothing should end up in stderr: '{proc.stderr}'")
        self.assertFalse(proc.stdout, f"Running with --quiet so nothing should end up in stdout: '{proc.stdout}'")
        self.assertEqual(1, stats.cacheable, msg=stats.print())
        self.assertEqual(0, stats.cache_hits, msg=stats.print())

        # changing back to the original contents should be a hit again
        stats.zero()
        self.TESTED_FILE.write_text(contents)
        print("Verifying after restoring...")
        self._run()
        self.assertEqual(1, stats.cacheable, msg=stats.print())
        self.assertEqual(1, stats.cache_hits, msg=stats.print())

    def test_include_modification(self):
        _cleanup()
        self._prepare_buildtree()
        self._fill_cache()

        stats = CCacheStats()

        # editing an included header should result in a cache miss
        tested_header = self.TESTED_FILE.with_suffix('.h')
        contents = tested_header.read_text()
        edited = contents.replace('<replace to edit>', 'testing')
        stats.zero()
        tested_header.write_text(edited)
        print("Verifying after modification...")
        proc = self._run()
        self.assertFalse(proc.stderr, f"Running with --quiet so nothing should end up in stderr: '{proc.stderr}'")
        self.assertFalse(proc.stdout, f"Running with --quiet so nothing should end up in stdout: '{proc.stdout}'")
        self.assertEqual(1, stats.cacheable, msg=stats.print())
        self.assertEqual(0, stats.cache_hits, msg=stats.print())

        # changing back to the original contents should be a hit again
        stats.zero()
        tested_header.write_text(contents)
        print("Verifying after restoring...")
        self._run()
        self.assertEqual(1, stats.cacheable, msg=stats.print())
        self.assertEqual(1, stats.cache_hits, msg=stats.print())

    def test_config_modification(self):
        _cleanup()
        self._prepare_buildtree()
        self._fill_cache()

        stats = CCacheStats()

        # editing the configuration should result in a cache miss
        contents = self.TESTED_CONFIG.read_text()
        edited = contents.replace('-llvm-header-guard', '-llvm-*')
        stats.zero()
        self.TESTED_CONFIG.write_text(edited)
        print("Verifying after modification...")
        self._run()
        self.assertEqual(1, stats.cacheable, msg=stats.print())
        self.assertEqual(0, stats.cache_hits, msg=stats.print())

        # changing back to the original contents should be a hit again
        stats.zero()
        self.TESTED_CONFIG.write_text(contents)
        print("Verifying after restoring...")
        self._run()
        self.assertEqual(1, stats.cacheable, msg=stats.print())
        self.assertEqual(1, stats.cache_hits, msg=stats.print())

    def test_param_modification(self):
        _cleanup()
        self._prepare_buildtree()
        self._fill_cache()

        stats = CCacheStats()

        # changing compiler flags should result in a cache miss
        contents = self.TESTED_PROJ.read_text()
        edited = contents.replace('# set(ENABLE_WARNING_AS_ERROR TRUE)', 'set(ENABLE_WARNING_AS_ERROR TRUE)')
        stats.zero()
        self.TESTED_PROJ.write_text(edited)
        print("Verifying after modification...")
        self._configure_buildtree(self.SRC_DIR, self.BUILD_DIR)
        self._run()
        self.assertEqual(1, stats.cacheable, msg=stats.print())
        self.assertEqual(0, stats.cache_hits, msg=stats.print())

        # changing back to the original contents should be a hit again
        stats.zero()
        self.TESTED_PROJ.write_text(contents)
        print("Verifying after restoring...")
        self._configure_buildtree(self.SRC_DIR, self.BUILD_DIR)
        self._run()
        self.assertEqual(1, stats.cacheable, msg=stats.print())
        self.assertEqual(1, stats.cache_hits, msg=stats.print())

    def test_error_logging(self):
        _cleanup()
        self._prepare_buildtree()
        self._fill_cache()

        stats = CCacheStats()

        # introducing an error should result in a cache miss and logged output
        contents = self.TESTED_FILE.read_text()
        edited = contents.replace('// insert unused variable here', 'int error = "string";')
        stats.zero()
        self.TESTED_FILE.write_text(edited)
        proc = self._run(check=False)
        self.assertNotEqual(0, proc.returncode)
        self.assertIn("error generated", proc.stderr, f"stderr: '{proc.stderr}'\nstdout: '{proc.stdout}'")
        self.assertIn("[clang-diagnostic-error]", proc.stdout, f"stderr: '{proc.stderr}'\nstdout: '{proc.stdout}'")
        self.assertEqual(1, stats.cacheable, msg=stats.print())
        self.assertEqual(0, stats.cache_hits, msg=stats.print())

    def test_with_extra_args(self):
        _cleanup()
        self._prepare_buildtree()

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
        self._prepare_buildtree()
        output = TestClangTidy.BUILD_DIR / 'clang-tidy.stamp'

        stats = CCacheStats()
        stats.zero()

        # first run should be cacheable but not in the cache yet
        self._run(extra_args=[f'-o={output.as_posix()}'])
        self.assertEqual(1, stats.cacheable, msg=stats.print())
        self.assertEqual(0, stats.cache_hits, msg=stats.print())
        self.assertTrue(output.exists())
        output.unlink()

        # second run should be served from the cache
        self._run(extra_args=[f'-o={output.as_posix()}'])
        self.assertEqual(2, stats.cacheable, msg=stats.print())
        self.assertEqual(1, stats.cache_hits, msg=stats.print())
        self.assertTrue(output.exists())

    def test_with_different_directories(self):
        _cleanup()
        base_dir0 = TestClangTidy.BUILD_DIR / '0'
        os.environ['CCACHE_BASEDIR'] = base_dir0.as_posix()
        build_dir0 = base_dir0 / 'build'
        src_dir0 = base_dir0 / 'src'
        self._prepare_buildtree(src_dir=src_dir0, build_dir=build_dir0)

        stats = CCacheStats()
        stats.zero()

        # first run should be cacheable but not in the cache yet
        print("Populate cache in dir 0...")
        self._run()
        self.assertEqual(1, stats.cacheable, msg=stats.print())
        self.assertEqual(0, stats.cache_hits, msg=stats.print())

        # second run in a different build path should be served from the cache
        print("Use cache in dir 1...")
        base_dir1 = TestClangTidy.BUILD_DIR / '1'
        os.environ['CCACHE_BASEDIR'] = base_dir1.as_posix()
        build_dir1 = base_dir1 / 'build'
        src_dir1 = base_dir1 / 'src'
        self._prepare_buildtree(src_dir=src_dir1, build_dir=build_dir1)
        self._run()
        self.assertEqual(2, stats.cacheable, msg=stats.print())
        self.assertEqual(1, stats.cache_hits, msg=stats.print())


if __name__ == '__main__':
    unittest.main()
