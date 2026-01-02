# MBR-DAQ-App [![C++20](https://img.shields.io/badge/C%2B%2B-20-blue?logo=c%2B%2B&logoColor=white)](https://en.cppreference.com/w/cpp/20.html) [![License](https://img.shields.io/github/license/watsonbw/MBR-DAQ-App)](LICENSE)
Data visualization and ESP32 interface for the University of Michigan's baja team.

## License
This project is licensed under the GNU General Public License v3.0 or later.

## Getting Started
Precompiled releases for macOS and windows are available in the releases tab. They are standalone executables and should work perfectly out of the box. You will likely have to fight your systems antivirus the first time you launch the app, since we are __not__ paying for code signing.
- Windows: Just have to tell Defender that it's ok.
- macOS: You have to remove the app from 'quarantine'. This can be done in settings, or by running this command in your terminal:
```shell
xattr -d com.apple.quarantine /path/to/baja-app
chmod +x /path/to/baja
```
- Linux: Currently not supported.

## Building From Source
### System Dependencies
Ensure you have the following system depencies:
- A C compiler (msys2's GCC is tested on windows, XCode's native clang compiler on mac)
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

This will build the fully optimized, standalone executable and place it in `build/dist` (relative to the project root). Please open an issue if you have any probles when compiling!

## Third-Party Software
This software statically links against FFmpeg, which was built with GPL components enabled.
- FFmpeg is licensed under the GNU General Public License.
- FFmpeg source code is available at https://ffmpeg.org
- Compilation information can be found [here](https://github.com/trevorswan11/ffmpeg)

This software builds the following from source:
- [Dear ImGUI](https://github.com/ocornut/imgui)
- [ImPlot](https://github.com/epezent/implot)
- [IXWebSocket](https://github.com/machinezone/IXWebSocket)
- [tinyfiledialogs](https://github.com/native-toolkit/libtinyfiledialogs)
- [OpenCV](https://github.com/opencv/opencv)
- [Sokol](https://github.com/floooh/sokol)
- [spdlog](https://github.com/gabime/spdlog)
- [stb](https://github.com/nothings/stb)

All third-party libraries listed above are licensed under terms compatible with the GNU GPL.

## Distribution
Copyright (C) 2026 Blake Watson & Trevor Swan

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

_No technical measures are used to prevent users from running modified versions of this software._