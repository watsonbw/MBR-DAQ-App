<h1 align="center">MBR-DAQ</h1>

<p align="center">
<img src="https://img.shields.io/badge/C%2B%2B-20-blue?logo=c%2B%2B&logoColor=white" alt="C++20" /> <a href="LICENSE"><img src="https://img.shields.io/github/license/watsonbw/MBR-DAQ-App" alt="License" /></a> <img src="https://img.shields.io/github/last-commit/watsonbw/MBR-DAQ-App" alt="Last Commit" /> <a href="https://github.com/watsonbw/MBR-DAQ-App/actions/workflows/format.yml"><img src="https://github.com/watsonbw/MBR-DAQ-App/actions/workflows/format.yml/badge.svg" alt="Formatting" /></a> <a href="https://github.com/watsonbw/MBR-DAQ-App/actions/workflows/ci.yml"><img src="https://github.com/watsonbw/MBR-DAQ-App/actions/workflows/ci.yml/badge.svg" alt="CI" /></a></a>
</p>

<p align="center">
Data visualization and ESP32 interface for the University of Michigan's Baja team.
</p>

## Availability

This software is provided precompiled for x86-64 Windows and Apple Silicone macOS only. Other platforms require manual tweaking of the build process and are not guaranteed to work.

On windows only, the app requires the included FFmpeg dll to be in its directory.

## Getting Started

Precompiled releases for macOS and windows are available in the releases tab. You will likely have to fight your systems antivirus the first time you launch the app, since we are **not** paying for code signing.

- Windows: Just have to tell Defender that it's ok.
- macOS: You have to remove the app from 'quarantine'. This can be done in settings, or by running this command in your terminal:

```shell
xattr -d com.apple.quarantine /path/to/baja-app
chmod +x /path/to/baja
```

- Linux: Currently not supported.

## Building From Source

If you are interested in becoming a contributor or simply want more information on compiling from source, check out [CONTRIBUTING.md](.github/CONTRIBUTING.md).

### System Dependencies

Ensure you have the following system dependencies:

- A C compiler (msys2's GCC & Clang are tested on windows, XCode's native clang compiler on mac)
- CMake (>=v3.31.5)
- A build system like Make or Ninja (Ninja is highly recommended)

### Compilation

Run the following command in a terminal with cmake and your compiler available in your system's Path:

```shell
git clone --recursive https://github.com/watsonbw/MBR-DAQ-App
cd MBR-DAQ-App
mkdir build
cd build
cmake -G Ninja ..
cmake --build . --target dist
```

This will build the fully optimized executable and place it in `build/dist` (relative to the project root). Please open an issue if you have any problems when compiling.

## Third-Party Software

This software dynamically links against a custom LGPL FFmpeg distribution provided by OpenCV.

- FFmpeg source code is available at https://ffmpeg.org

This software builds the following from source:

- [Dear ImGUI](https://github.com/ocornut/imgui)
- [ImPlot](https://github.com/epezent/implot)
- [IXWebSocket](https://github.com/machinezone/IXWebSocket)
- [tinyfiledialogs](https://github.com/native-toolkit/libtinyfiledialogs)
- [OpenCV](https://github.com/opencv/opencv)
- [Sokol](https://github.com/floooh/sokol)
- [spdlog](https://github.com/gabime/spdlog)
- [stb](https://github.com/nothings/stb)
- [taglib](https://github.com/taglib/taglib)
- [utfcpp](https://github.com/nemtrif/utfcpp)

All third-party libraries listed above are licensed under terms compatible with the MIT License.

## Distribution

Copyright (C) 2026 Blake Watson & Trevor Swan

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

_No technical measures are used to prevent users from running modified versions of this software._
