# Macro to add a third-party vendor subdirectory safely with warning suppression
macro(add_vendor_subdirectory dir)
    cmake_policy(PUSH)
    cmake_policy(SET CMP0077 NEW)
    set(CMAKE_C_FLAGS_BACKUP "${CMAKE_C_FLAGS}")
    set(CMAKE_CXX_FLAGS_BACKUP "${CMAKE_CXX_FLAGS}")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${WARNING_IGNORE}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${WARNING_IGNORE}")

    add_subdirectory(${dir})

    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS_BACKUP}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_BACKUP}")
    cmake_policy(POP)
endmacro()

# nlohmann/json
set(JSON_BuildTests OFF CACHE BOOL "" FORCE)
set(JSON_Install OFF CACHE BOOL "" FORCE)
set(JSON_MultipleHeaders OFF CACHE BOOL "" FORCE)
add_vendor_subdirectory(vendor/json)

# Microsoft GSL
add_vendor_subdirectory(vendor/gsl)

# {fmt}
add_vendor_subdirectory(vendor/fmt)

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
        "-framework AVFoundation"
        "-framework CoreMedia"
        "-framework CoreVideo"
    )
elseif(WIN32)
    target_compile_definitions(sokol_setup PUBLIC SOKOL_D3D11)
    target_link_libraries(sokol_setup INTERFACE d3d11 dxgi dwmapi)
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
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Static linking" FORCE)
set(BUILD_opencv_apps OFF CACHE BOOL "Classifier training not needed" FORCE)
set(BUILD_opencv_python2 OFF CACHE BOOL "Disable python support" FORCE)
set(BUILD_opencv_python3 OFF CACHE BOOL "Disable python support" FORCE)
set(BUILD_EXAMPLES OFF CACHE BOOL "Examples not needed" FORCE)
set(BUILD_DOCS OFF CACHE BOOL "OpenCV docs are not needed" FORCE)
set(BUILD_TESTS OFF CACHE BOOL "Testing not needed for vendoring" FORCE)
set(BUILD_PERF_TESTS OFF CACHE BOOL "Testing not needed for vendoring" FORCE)
set(BUILD_CUDA_STUBS OFF CACHE BOOL "Cuda is never needed" FORCE)
set(WITH_CUDA OFF CACHE BOOL "" FORCE)
set(BUILD_JAVA OFF CACHE BOOL "" FORCE)
set(WITH_IPP OFF CACHE BOOL "Intel IPP support not needed" FORCE)
set(WITH_ITT OFF CACHE BOOL "Intel ITT support not needed" FORCE)
set(WITH_OPENCL OFF CACHE BOOL "" FORCE)
set(WITH_WEBP OFF CACHE BOOL "" FORCE)
set(WITH_OPENEXR OFF CACHE BOOL "" FORCE)

set(OPENCV_WARNINGS_ARE_ERRORS OFF CACHE BOOL "" FORCE)
set(CMAKE_SUPPRESS_DEVELOPER_WARNINGS ON CACHE BOOL "" FORCE)

if(APPLE)
    set(WITH_FFMPEG OFF CACHE BOOL "System dependencies instead" FORCE)
    set(HAVE_FFMPEG OFF CACHE BOOL "System dependencies instead" FORCE)
    set(WITH_AVFOUNDATION ON CACHE BOOL "Apple framework is statically linked" FORCE)
    unset(FFMPEG_LIBRARIES CACHE)
    unset(FFMPEG_INCLUDE_DIRS CACHE)
endif()

add_vendor_subdirectory(vendor/opencv)

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
add_vendor_subdirectory(vendor/spdlog)

get_target_property(SPDLOG_INCLUDE_DIRS spdlog INTERFACE_INCLUDE_DIRECTORIES)
set_target_properties(spdlog PROPERTIES
    INTERFACE_SYSTEM_INCLUDE_DIRECTORIES "${SPDLOG_INCLUDE_DIRS}"
)

# IXWebSocket
set(IXWEBSOCKET_INSTALL OFF CACHE BOOL "Just linking statically" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(USE_TLS OFF CACHE BOOL "" FORCE)
set(USE_WS OFF CACHE BOOL "" FORCE)
set(USE_MBED_TLS OFF CACHE BOOL "" FORCE)
set(USE_OPEN_SSL OFF CACHE BOOL "" FORCE)
set(USE_ZLIB OFF CACHE BOOL "" FORCE)
add_vendor_subdirectory(vendor/ixwebsocket)

# stb_image
add_library(stb_image_setup STATIC src/vendor/stb_impl.cpp)
target_include_directories(stb_image_setup SYSTEM PUBLIC vendor/stb)
target_compile_options(stb_image_setup PRIVATE ${WARNING_IGNORE})

# taglib
set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(WITH_ZLIB OFF CACHE BOOL "" FORCE)
add_vendor_subdirectory(vendor/taglib)

target_include_directories(tag
    PRIVATE
        "${CMAKE_SOURCE_DIR}/vendor/taglib/taglib"
        "${CMAKE_SOURCE_DIR}/vendor/taglib/taglib/toolkit"
    INTERFACE
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/vendor/taglib>"
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/vendor/taglib/taglib>"
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/vendor/taglib/taglib/toolkit>"
)

# Serial Library
set(SERIAL_DIR "vendor/serial")
set(SERIAL_SOURCES
    ${SERIAL_DIR}/src/serial.cc
)

if(APPLE)
    list(APPEND SERIAL_SOURCES
        ${SERIAL_DIR}/src/impl/unix.cc
        ${SERIAL_DIR}/src/impl/list_ports/list_ports_osx.cc
    )
elseif(WIN32)
    list(APPEND SERIAL_SOURCES
        ${SERIAL_DIR}/src/impl/win.cc
        ${SERIAL_DIR}/src/impl/list_ports/list_ports_win.cc
    )
endif()

add_library(serial_setup STATIC ${SERIAL_SOURCES})
target_include_directories(serial_setup PUBLIC
    ${SERIAL_DIR}/include
)
target_compile_options(serial_setup PRIVATE ${WARNING_IGNORE})

if(APPLE)
    target_link_libraries(serial_setup INTERFACE
        "-framework Foundation"
        "-framework IOKit"
    )
elseif(WIN32)
    target_link_libraries(serial_setup INTERFACE setupapi)
endif()
