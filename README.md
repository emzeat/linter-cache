Wrapper to invoke code linters through ccache to accelerate analysis as part of a regular compile job.

Use by simply putting `linter-cache` before the actual linting command.

This was originally started as [cache-tidy.py in mz-cmaketools](https://github.com/emzeat/mz-cmaketools/blob/master/cache-tidy.py)
but moved to a dedicated C++ project as ccache expects compilers to have .exe and not .py as extension on Windows
and the startup times for python were found to be too slow sometimes.

While there is other caching implementations like [cltcache](https://github.com/freedick/cltcache)
or [clang-tidy-cache](https://github.com/ejfitzgerald/clang-tidy-cache) these
have the downside that they create separate caches. The goal of this project is
to achieve acceleration through caching but reusing the setup already made for
ccache including its remote capabilities.

Currently supported and tested linters:
* clang-tidy
