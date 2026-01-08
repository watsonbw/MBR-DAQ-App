# Contributing Overview

All code submitted through PR's must pass all CI checks. Code that does not compile or pass formatting/static analysis checks will be sent back for modifications.

Please try to open issues where possible to better describe the problem your PR is addressing. This also makes it easy for us to mention or reopen issues that had thought to have been resolved. Issue templates are provided and should be used whenever possible, and should be made as descriptive as possible.

The `dev` branch is where all active development occurs. All Pull Requests should start by branching off of `dev` and should be submitted against this branch, _not_ `main`.

Contributors, upon a successful merge of their PR, will have the name or github username added to the AUTHORS.md file shipped with every release. You can opt out of this by letting a maintainer know.

# Toolchain

Releases are compiled with Apple Clang on macOS and GCC 15 on Windows through the msys2 shell. A good way to speed up the review process is to use these tools. Using these toolchains is recommended to ensure your changes play nicely with the project's build steps. Clang is also used on windows in the form of clang-cl (which uses the lld-link linker), but this is never tested in CI runs.

MSVC is not directly supported on windows, but clang-cl can be used to serve as a GCC-like frontend if you are interested. All ABI issues are handled for you regardless! Linux is not and will never be strictly supported as the project's goals do not require Linux compatibility. You may find that you can build and run the project on your Linux machine, but please do not submit issues relating to Linux compatibility issues.

## Installing Dependencies

This project uses CMake, and it is highly recommended that you use the Ninja build system to speed up builds. The following commands assume you have [homebrew](https://brew.sh/) available in your path on macOS and [msys2](https://www.msys2.org/) on Windows.

### MacOS

```shell
xcode-select --install
brew update
brew install llvm cmake ninja
```

While LLVM's compiler should not be used on macos, the included formatter and linter are.

### Windows

```shell
pacman -S mingw-w64-ucrt-x86_64-gcc
pacman -S mingw-w64-ucrt-x86_64-clang-tools-extra
pacman -S mingw-w64-ucrt-x86_64-cmake
pacman -S mingw-w64-ucrt-x86_64-ninja
```

You may need to add the `ucrt/bin` folder to your path, but this is system dependent based on how you configured msys2. You can also simplify this process by running all build commands through the `MSYS2 UCRT64` shortcut installed with msys2.

## Compiling

This project has many third party (vendored) libraries. These libraries are vendored in through submodules, so you must append `--recursive` to your initial `git clone` command: `git clone --recursive https://github.com/watsonbw/MBR-DAQ-App`.

If you failed to clone recursively or notice that submodules have been updated, simply run `git submodule update --init --recursive`.

Most major work happens on the `dev` branch. This is a very unstable branch and is not guaranteed to be in a compile-able state at all times. For your first compilation, it is recommended that you build from the main branch.

Once you have the repository checked out as well as all system dependencies installed, simply run:

```shell
mkdir -p build && cd build
cmake -G Ninja ..
cmake --build .
```

This will take a very long time the first time around, as it is determining system dependency installation status and compiling everything from scratch. Once finished, this will produce all project configuration executables, placing them in their relevant directories.

# Formatting & Static Analysis

Please ensure all code is formatted according to `clang-format`. As `clang-format` formatting is not guaranteed to be the same across tool versions, we recommend you use the clang-format shipped with `llvm-21`. The auto format check uses this version and runs on macOS.

Code must also past all static analysis checks. This is considerably more important as these checks enforce style conformance and reduce bugs proactively. As with formatting, the automated check runs on macOS and uses `run-clang-tidy`, as provided by `llvm-21`.

# Testing

Tests should be written for anything that has considerable weight. This is a subjective measurement. If you believe that something is volatile (changes to a related system will break behavior) then a test should be written. That being said, if a block of code is tested by another test in the codebase, you need to not worry about writing a specific test for it. You also need not concern yourself with code that is practically infallible due to compiler intrinsics, dead-simple functions, etc. Strictly GUI-facing code does not need to be tested. When possible, break up functions into smaller bits that can be more easily unit tested. Please note that macOS will also test for memory leaks during CI. Memory is a resource that must be managed carefully.

# Packaging

Packaging is done automatically through GitHub actions. Do not concern yourself with packaging a release on your own machine.

# Collaborators

If you become a collaborator with push access, you are expected to work on a branch that is _not_ the `main` branch. Every push to the main branch causes the actions runners to spin up for the CI runs, which can take up to 15 minutes. The main branch should be used strictly for releases.
