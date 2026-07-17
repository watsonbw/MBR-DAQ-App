cmake_minimum_required(VERSION 3.20)
set(CMAKE_COLOR_MAKEFILE OFF)

# Platform detection & compiler language selection
if(APPLE)
    set(LANGUAGES C CXX OBJCXX)
elseif(WIN32)
    set(LANGUAGES C CXX)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
else()
    message(ERROR "Platform currently unsupported!")
endif()

set(CMAKE_BUILD_TYPE Release)
project(MBR-DAQ
    VERSION 0.1.0
    LANGUAGES ${LANGUAGES}
)

# Qualify compile commands database for tooling
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Configure CMake module path for local scripts
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

# Include compiler flags & options configuration
include(CompilerFlags)

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

# Include all third-party dependencies setup
include(VendorSetup)

# Include build suite target functions
include(MbrDaqSuite)

# Standard building & testing targets
create_standard_target("${DIST_FLAGS}" "dist")
create_standard_target("${RELEASE_FLAGS}" "release")
create_standard_target("${DEBUG_FLAGS}" "debug")

enable_testing()
add_test(NAME run_tests_debug COMMAND mbrdaq_tests_debug)
add_test(NAME run_tests_release COMMAND mbrdaq_tests_release)
add_test(NAME run_tests_dist COMMAND mbrdaq_tests_dist)

# Compiler specific configuration
add_custom_target(mbrdaq DEPENDS dist release debug)

add_custom_target(qtest
    DEPENDS mbrdaq_tests_debug
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure -R run_tests_debug
    USES_TERMINAL)

add_custom_target(mbrdaq_tests
    DEPENDS mbrdaq_tests_debug mbrdaq_tests_release mbrdaq_tests_dist
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure -R run_tests_debug
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure -R run_tests_release
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure -R run_tests_dist
    USES_TERMINAL)

# Include static analysis & format tooling targets
include(Tooling)
