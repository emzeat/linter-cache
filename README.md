Wrapper to invoke code linters through ccache to accelerate analysis as part of a regular compile job. Use as if running the linter directly.

While there is other caching implementations like [cltcache](https://github.com/freedick/cltcache)
or [clang-tidy-cache](https://github.com/ejfitzgerald/clang-tidy-cache) these
have the downside that they create separate caches. The goal of this project is
to achieve acceleration through caching but reusing the setup already made for
ccache including its remote capabilities.

Currently supported and tested linters:
* clang-tidy
