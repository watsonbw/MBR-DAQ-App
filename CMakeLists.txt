cmake_minimum_required(VERSION 3.20)
set(CMAKE_COLOR_MAKEFILE OFF)

# Global configuration
set(GLOBAL_NAME baja_data)
set(GLOBAL_VERSION 0.1.0)
set(CMAKE_BUILD_TYPE Release)

if(APPLE)
    project(${GLOBAL_NAME}
    VERSION ${GLOBAL_VERSION}
    LANGUAGES C CXX OBJCXX
)
elseif(WIN32)
    project(${GLOBAL_NAME}
    VERSION ${GLOBAL_VERSION}
    LANGUAGES C CXX
)
else()
    message(ERROR "Platform currently unsupported!")
endif()

# Qualify compile commands database for tooling
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(APPLE)
    execute_process(
    COMMAND xcrun --show-sdk-path
    OUTPUT_VARIABLE CMAKE_OSX_SYSROOT
    OUTPUT_STRIP_TRAILING_WHITESPACE)
elseif(WIN32)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
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
    "-DCV_IGNORE_DEBUG_BUILD_GUARD"
)

# Source file collection
set(SRC_DIR "${CMAKE_SOURCE_DIR}/src")
file(GLOB_RECURSE ALL_SOURCES "${SRC_DIR}/*.cpp")
set(INCLUDE_DIR "${CMAKE_SOURCE_DIR}/include")
file(GLOB_RECURSE HEADERS "${INCLUDE_DIR}/*.hpp" "${INCLUDE_DIR}/*.h")

set(LIB_SOURCES ${ALL_SOURCES})
list(FILTER LIB_SOURCES EXCLUDE REGEX "main\\.cpp$")

set(TEST_DIR "${CMAKE_SOURCE_DIR}/tests")
file(GLOB_RECURSE TEST_SOURCES "${TEST_DIR}/*.cpp")
set(FRAMEWORK_REGEX "test_framework/*")
list(FILTER TEST_SOURCES EXCLUDE REGEX ${FRAMEWORK_REGEX})

# Sokol
add_library(sokol_setup STATIC src/vendor/sokol_impl.cpp)
target_include_directories(sokol_setup SYSTEM PUBLIC
    vendor/sokol
    vendor/sokol/util
    vendor/imgui
    vendor/imgui/backends)
target_compile_options(sokol_setup PRIVATE ${WARNING_IGNORE})

if(APPLE)
    target_compile_definitions(sokol_setup PUBLIC SOKOL_METAL)
    set_source_files_properties(src/vendor/sokol_impl.cpp PROPERTIES LANGUAGE OBJCXX)
    target_link_libraries(sokol_setup INTERFACE
        "-framework Cocoa"
        "-framework IOKit"
        "-framework Metal"
        "-framework MetalKit"
        "-framework QuartzCore"
    )
elseif(WIN32)
    target_compile_definitions(sokol_setup PUBLIC SOKOL_D3D11)
    target_link_libraries(sokol_setup INTERFACE d3d11 dxgi)
endif()

# Dear ImGUI
add_library(imgui_setup STATIC
    vendor/imgui/imgui.cpp
    vendor/imgui/imgui_draw.cpp
    vendor/imgui/imgui_widgets.cpp
    vendor/imgui/imgui_tables.cpp
    vendor/imgui/imgui_demo.cpp
)
target_include_directories(imgui_setup SYSTEM PUBLIC
    vendor/imgui vendor/imgui/backends)
target_link_libraries(imgui_setup PUBLIC sokol_setup)

# ImPlot
add_library(implot_setup STATIC
    vendor/implot/implot.cpp
    vendor/implot/implot_items.cpp
    vendor/implot/implot_demo.cpp
)
target_include_directories(implot_setup SYSTEM PUBLIC vendor/implot)
target_link_libraries(implot_setup PUBLIC imgui_setup)
target_compile_options(implot_setup PRIVATE ${WARNING_IGNORE})

# OpenCV
set(BUILD_LIST "core,imgproc,imgcodecs,videoio,highgui" CACHE STRING "" FORCE)
set(BUILD_opencv_apps OFF CACHE BOOL "" FORCE)
set(BUILD_opencv_java OFF CACHE BOOL "" FORCE)
set(BUILD_opencv_python OFF CACHE BOOL "" FORCE)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(BUILD_PERF_TESTS OFF CACHE BOOL "" FORCE)
set(WITH_IPP OFF CACHE BOOL "" FORCE)
set(WITH_ITT OFF CACHE BOOL "" FORCE)
set(WITH_CUDA OFF CACHE BOOL "" FORCE)
set(WITH_OPENCL OFF CACHE BOOL "" FORCE)
set(WITH_WEBP OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

set(OPENCV_WARNINGS_ARE_ERRORS OFF CACHE BOOL "" FORCE)
set(CMAKE_SUPPRESS_DEVELOPER_WARNINGS ON CACHE BOOL "" FORCE)

cmake_policy(PUSH)
cmake_policy(SET CMP0077 NEW)

set(CMAKE_C_FLAGS_BACKUP "${CMAKE_C_FLAGS}")
set(CMAKE_CXX_FLAGS_BACKUP "${CMAKE_CXX_FLAGS}")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${WARNING_IGNORE}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${WARNING_IGNORE}")

# ffmpeg resolution
set(_ffmpeg_libs avcodec avformat avutil swscale swresample)

if(APPLE)
    set(FFMPEG_DIR "${CMAKE_SOURCE_DIR}/vendor/ffmpeg/mac")
    set(FFMPEG_INC "${CMAKE_SOURCE_DIR}/vendor/ffmpeg/include")
    set(FFMPEG_LIBRARIES "")
    
    foreach(_lib ${_ffmpeg_libs})
        add_library(ffmpeg_${_lib} SHARED IMPORTED)
    
            set_target_properties(ffmpeg_${_lib} PROPERTIES
                IMPORTED_LOCATION "${FFMPEG_DIR}/lib${_lib}.dylib"
            )
        
        set_target_properties(ffmpeg_${_lib} PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${FFMPEG_INC}"
        )
        list(APPEND FFMPEG_LIBRARIES ffmpeg_${_lib})
    endforeach()

    set(WITH_FFMPEG ON CACHE BOOL "" FORCE)
    set(HAVE_FFMPEG ON CACHE BOOL "" FORCE)
    set(FFMPEG_INCLUDE_DIRS "${FFMPEG_INC}" CACHE PATH "" FORCE)
    set(FFMPEG_LIBRARIES "${FFMPEG_LIBRARIES}" CACHE STRING "" FORCE)
    
    set(OPENCV_FFMPEG_SKIP_BUILD_CHECK ON CACHE BOOL "" FORCE)
    set(OPENCV_FFMPEG_SKIP_VERSION_CHECK ON CACHE BOOL "" FORCE)
endif()

add_subdirectory(vendor/opencv)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS_BACKUP}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_BACKUP}")

cmake_policy(POP)

# TinyFileDialogs
add_library(tinyfiledialogs STATIC vendor/libtinyfiledialogs/tinyfiledialogs.c)
target_include_directories(tinyfiledialogs PUBLIC vendor/libtinyfiledialogs)
target_compile_options(tinyfiledialogs PRIVATE ${WARNING_IGNORE})
if(WIN32)
    target_link_libraries(tinyfiledialogs PUBLIC comdlg32 ole32 user32 shell32)
endif()

# spdlog
set(SPDLOG_BUILD_SHARED OFF CACHE BOOL "" FORCE)
set(SPDLOG_BUILD_EXAMPLE OFF CACHE BOOL "" FORCE)
set(SPDLOG_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(SPDLOG_BUILD_BENCH OFF CACHE BOOL "" FORCE)
set(SPDLOG_INSTALL OFF CACHE BOOL "" FORCE)

cmake_policy(PUSH)
cmake_policy(SET CMP0077 NEW)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${WARNING_IGNORE}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${WARNING_IGNORE}")

add_subdirectory(vendor/spdlog)

get_target_property(SPDLOG_INCLUDE_DIRS spdlog INTERFACE_INCLUDE_DIRECTORIES)
set_target_properties(spdlog PROPERTIES
    INTERFACE_SYSTEM_INCLUDE_DIRECTORIES "${SPDLOG_INCLUDE_DIRS}"
)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS_BACKUP}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_BACKUP}")

cmake_policy(POP)

# IXWebSocket
set(IXWEBSOCKET_INSTALL OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(USE_TLS OFF CACHE BOOL "" FORCE)
set(USE_WS OFF CACHE BOOL "" FORCE)
set(USE_MBED_TLS OFF CACHE BOOL "" FORCE)
set(USE_OPEN_SSL OFF CACHE BOOL "" FORCE)
set(USE_ZLIB OFF CACHE BOOL "" FORCE)

cmake_policy(PUSH)
cmake_policy(SET CMP0077 NEW)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${WARNING_IGNORE}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${WARNING_IGNORE}")

add_subdirectory(vendor/ixwebsocket)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS_BACKUP}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_BACKUP}")

cmake_policy(POP)

# stb_image
add_library(stb_image_setup STATIC src/vendor/stb_impl.cpp)
target_include_directories(stb_image_setup SYSTEM PUBLIC vendor/stb)
target_compile_options(stb_image_setup PRIVATE ${WARNING_IGNORE})

function(create_baja_suite TARGET_NAME EXTRA_FLAGS OUT_DIR)
    add_library(baja_core_${TARGET_NAME} STATIC ${LIB_SOURCES})
    target_link_libraries(baja_core_${TARGET_NAME} PUBLIC
        imgui_setup
        implot_setup
        ${FFMPEG_LIBRARIES}
        tinyfiledialogs
        opencv_core
        opencv_imgproc
        opencv_imgcodecs
        opencv_videoio
        opencv_highgui
        spdlog::spdlog
        ixwebsocket
        stb_image_setup
    )

    target_include_directories(baja_core_${TARGET_NAME} PUBLIC
        include)

    target_include_directories(baja_core_${TARGET_NAME} SYSTEM PUBLIC
        ${CMAKE_BINARY_DIR}
        ${CMAKE_SOURCE_DIR}/vendor/opencv/include
        ${CMAKE_SOURCE_DIR}/vendor/opencv/modules/core/include
        ${CMAKE_SOURCE_DIR}/vendor/opencv/modules/imgproc/include
        ${CMAKE_SOURCE_DIR}/vendor/opencv/modules/imgcodecs/include
        ${CMAKE_SOURCE_DIR}/vendor/opencv/modules/videoio/include
        ${CMAKE_SOURCE_DIR}/vendor/opencv/modules/highgui/include
        ${FFMPEG_INC}
    )

    target_compile_options(baja_core_${TARGET_NAME} PRIVATE ${BASE_FLAGS})

    target_compile_options(baja_core_${TARGET_NAME} PRIVATE ${EXTRA_FLAGS})
    target_link_options(baja_core_${TARGET_NAME} PRIVATE ${EXTRA_FLAGS})
    set_target_properties(baja_core_${TARGET_NAME}
        PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${OUT_DIR})

    # Fresh catch2 configuration per target
    add_library(catch2_${TARGET_NAME}
        STATIC tests/test_framework/catch_amalgamated.cpp)

    target_include_directories(catch2_${TARGET_NAME}
        PUBLIC tests/test_framework)

    target_compile_options(catch2_${TARGET_NAME}
        PRIVATE ${WARNING_IGNORE})

    set_target_properties(catch2_${TARGET_NAME}
        PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${OUT_DIR})

    add_executable(baja_tests_${TARGET_NAME} ${TEST_SOURCES})
    target_link_libraries(baja_tests_${TARGET_NAME}
        PRIVATE catch2_${TARGET_NAME} baja_core_${TARGET_NAME})
    target_include_directories(baja_tests_${TARGET_NAME}
        PRIVATE include tests/helpers)
    target_compile_options(baja_tests_${TARGET_NAME}
        PRIVATE ${WARNING_IGNORE})

    target_compile_options(baja_tests_${TARGET_NAME}
        PRIVATE ${EXTRA_FLAGS})
    target_link_options(baja_tests_${TARGET_NAME} PRIVATE ${EXTRA_FLAGS})
    set_target_properties(baja_tests_${TARGET_NAME}
        PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${OUT_DIR})
endfunction()

function(create_standard_target EXTRA_FLAGS TARGET_NAME)
    create_baja_suite("${TARGET_NAME}"
        "${EXTRA_FLAGS}"
        "${CMAKE_BINARY_DIR}/${TARGET_NAME}")

    add_executable(baja_${TARGET_NAME} src/main.cpp)
    target_link_libraries(baja_${TARGET_NAME}
        PRIVATE baja_core_${TARGET_NAME} imgui_setup tinyfiledialogs)
    add_custom_target(${TARGET_NAME} DEPENDS
        baja_tests_${TARGET_NAME} baja_${TARGET_NAME})

    set_target_properties(baja_${TARGET_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${TARGET_NAME}
        OUTPUT_NAME baja)

    if(APPLE)
        set_target_properties(baja_${TARGET_NAME} PROPERTIES
            BUILD_WITH_INSTALL_RPATH TRUE
            INSTALL_RPATH "@executable_path"
        )

        add_custom_command(TARGET baja_${TARGET_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${CMAKE_SOURCE_DIR}/vendor/ffmpeg/mac"
                "$<TARGET_FILE_DIR:baja_${TARGET_NAME}>"
            
            COMMAND bash -c "cd \"$<TARGET_FILE_DIR:baja_${TARGET_NAME}>\" && for lib in *.dylib; do [ -L \"$lib\" ] && continue; install_name_tool -id \"@rpath/$lib\" \"$lib\" 2>/dev/null; install_name_tool -change \"${FFMPEG_DIR}/$lib\" \"@rpath/$lib\" \"$<TARGET_FILE:baja_${TARGET_NAME}>\" 2>/dev/null || true; done"
            VERBATIM
            COMMENT "Remapping FFMPEG dylibs"
        )
    elseif(WIN32)
        add_custom_command(TARGET baja_${TARGET_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${CMAKE_BINARY_DIR}/bin"
                "$<TARGET_FILE_DIR:baja_${TARGET_NAME}>"
            COMMENT "Copy FFMPEG to output directory"
        )
    endif()
endfunction()

# Standard building & testing targets
create_standard_target("${DIST_FLAGS}" "dist")
create_standard_target("${RELEASE_FLAGS}" "release")
create_standard_target("${DEBUG_FLAGS}" "debug")

enable_testing()
add_test(NAME run_tests_debug COMMAND baja_tests_debug)
add_test(NAME run_tests_release COMMAND baja_tests_release)
add_test(NAME run_tests_dist COMMAND baja_tests_dist)

# Compiler specific configuration
add_custom_target(baja DEPENDS dist release debug)

add_custom_target(qtest
    DEPENDS baja_tests_debug
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure -R run_tests_debug
    USES_TERMINAL)

add_custom_target(baja_tests
    DEPENDS baja_tests_debug baja_tests_release baja_tests_dist
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure -R run_tests_debug
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure -R run_tests_release
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure -R run_tests_dist
    USES_TERMINAL)

# Standard tooling disabled on windows due to clang/llvm weirdness
if(APPLE)
    if(CMAKE_C_COMPILER_ID MATCHES "Clang")
        set(ASAN_FLAGS "-fsanitize=undefined" "-fno-sanitize-recover=all" "-g")
        set(ASAN_RUN_CMD leaks --atExit -- $<TARGET_FILE:baja_tests_asan>)
    elseif(CMAKE_C_COMPILER_ID STREQUAL "GNU")
        message(ERROR "The GNU Compiler is not supported on macOS")
    endif()

    create_baja_suite("asan" "${ASAN_FLAGS}" "${CMAKE_BINARY_DIR}/asan")
    add_custom_target(asan
        COMMAND ${ASAN_RUN_CMD}
        DEPENDS baja_tests_asan
        USES_TERMINAL)
endif()

# Tooling
set(ESP32_DIR "${CMAKE_SOURCE_DIR}/boardcode")

file(GLOB_RECURSE TOOLING_SOURCES
    ${SRC_DIR}/*.cpp
    ${INCLUDE_DIR}/*.hpp
    ${TEST_DIR}/*.cpp
    ${TEST_DIR}/*.hpp
    ${ESP32_DIR}/*.hpp
    ${ESP32_DIR}/*.cpp
    ${ESP32_DIR}/*.ino
)
list(FILTER TOOLING_SOURCES EXCLUDE REGEX ${FRAMEWORK_REGEX})

# Formatting
find_program(CLANG_FORMAT clang-format)
if(CLANG_FORMAT)
    add_custom_target(fmt COMMAND ${CLANG_FORMAT} -i ${TOOLING_SOURCES})
    add_custom_target(fmt-check COMMAND
        ${CLANG_FORMAT}
        --dry-run
        --Werror
        ${TOOLING_SOURCES})
endif()

# Cloc
find_program(CLOC cloc)
if (CLOC)
    add_custom_target(cloc COMMAND ${CLOC} ${TOOLING_SOURCES})
endif()
