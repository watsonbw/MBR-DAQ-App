if(APPLE)
    execute_process(
    COMMAND xcrun --show-sdk-path
    OUTPUT_VARIABLE CMAKE_OSX_SYSROOT
    OUTPUT_STRIP_TRAILING_WHITESPACE)
elseif(WIN32)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
endif()

if(MINGW)
    add_link_options("-static-libgcc" "-static-libstdc++" "-static")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(BASE_FLAGS "-Wall" "-Wextra" "-Werror" "-Wpedantic")
set(WARNING_IGNORE "-w")

set(DIST_FLAGS "-O3" "-DNDEBUG" "-DDIST")
set(RELEASE_FLAGS "-O2" "-DRELEASE")
set(DEBUG_FLAGS
    "-O0"
    "-g"
    "-DDEBUG"
    "-UNDEBUG"
    "-DLOGGING"
    "-DCV_IGNORE_DEBUG_BUILD_GUARD"
)
