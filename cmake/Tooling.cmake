# Standard tooling disabled on windows due to clang/llvm weirdness
if(APPLE)
    if(CMAKE_C_COMPILER_ID MATCHES "Clang")
        set(ASAN_FLAGS "-fsanitize=undefined" "-fno-sanitize-recover=all" "-g")
        set(ASAN_RUN_CMD leaks --atExit -- $<TARGET_FILE:mbrdaq_tests_asan>)
    else()
        message(ERROR "Cannot run asan with non-clang compilers on macOS")
    endif()

    create_mbrdaq_suite("asan" "${ASAN_FLAGS}" "${CMAKE_BINARY_DIR}/asan")
    add_custom_target(asan
        COMMAND ${ASAN_RUN_CMD}
        DEPENDS mbrdaq_tests_asan
        USES_TERMINAL)
endif()

# Tooling
file(GLOB_RECURSE TOOLING_SOURCES
    ${SRC_DIR}/*.cpp
    ${INCLUDE_DIR}/*.hpp
    ${TEST_DIR}/*.cpp
    ${TEST_DIR}/*.hpp
)
list(FILTER TOOLING_SOURCES EXCLUDE REGEX ${FRAMEWORK_REGEX})

# Formatting
find_program(CLANG_FORMAT clang-format)
if(CLANG_FORMAT)
    add_custom_target(format COMMAND ${CLANG_FORMAT} -i ${TOOLING_SOURCES})
    add_custom_target(format-check COMMAND
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

# Clang tidy has a tough time on windows
if(APPLE)
    file(GLOB_RECURSE TIDY_SOURCES
        ${SRC_DIR}/*.cpp
        ${INCLUDE_DIR}/*.hpp
        ${TEST_DIR}/*.cpp
        ${TEST_DIR}/*.hpp
    )
    list(FILTER TIDY_SOURCES EXCLUDE REGEX ${FRAMEWORK_REGEX})

    find_program(RUN_CLANG_TIDY run-clang-tidy)
    find_program(CLANG_TIDY clang-tidy)
    if(RUN_CLANG_TIDY)
        add_custom_target(tidy COMMAND
            ${RUN_CLANG_TIDY}
            -p ${CMAKE_BINARY_DIR}
            ${TIDY_SOURCES})
    elseif(CLANG_TIDY)
        add_custom_target(tidy COMMAND
            ${CLANG_TIDY}
            -p ${CMAKE_BINARY_DIR}
            ${TIDY_SOURCES})
    endif()
endif()
