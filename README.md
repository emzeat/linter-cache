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

## Table of Contents

- [Requirements](#requirements)
- [Usage](#usage)
- [Contributing](#contributing)
- [Acknowledgements](#acknowledgements)
- [License](#license)

## Requirements

CMake is used as the build system wrapped with my [mz-cmaketools](https://emzeat.de/mz-cmaketools) for convenience.

Dependencies get managed through conan 1.x, version 2 was not tested yet.

Make sure you have `cmake`, `conan`, `g++` and `ninja` installed and available in your path. Use cmake to configure and build the project:
```bash
mkdir build/release
cd build/release
cmake -G Ninja -D CMAKE_BUILD_TYPE=Release -S ../.. -B .
cmake --build .
```

The resulting binaries can be found in the `bin` directory of the build folder created above.

## Usage

Simply replace your calls to `clang-tidy` in your build scripts with calls to the
`linter-cache` binary and pass `--clang-tidy <your clang-tidy path>`
to switch to clang-tidy mode and have your linting cached.

For example a call `clang-tidy -p _build/compile_commands.json src/main.cpp` becomes
`linter-cache --clang-tidy=clang-tidy -p _build/compile_commands.json src/main.cpp`

## Contributing

We welcome any contributions.

We use GitHub to handle bug reports, feature requests and questions - please use "issues" for any of these. Code changes
and documentation improvements get handled via pull requests.

In order to foster open exchange, all communication should happen
using the English language.

Any changes to this repository get done via GitHub's standard workflow. See GitHub's [First Contributions document](https://github.com/firstcontributions/first-contributions) for an introduction.


## Acknowledgements

This project is using [mz-lictools](https://github.com/emzeat/mz-lictools) to
manage all license headers which will also automatically add an attribution
to the top of a file you edit.

Also see the [list of contributors](https://github.com/emzeat/linter-cache/graphs/contributors) generated by Github.

## License

This project is licensed under the [Apache 2.0 License](LICENSE).
