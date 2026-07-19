function(create_mbrdaq_suite TARGET_NAME EXTRA_FLAGS OUT_DIR)
    add_library(mbrdaq_core_${TARGET_NAME} STATIC ${LIB_SOURCES})
    target_link_libraries(mbrdaq_core_${TARGET_NAME} PUBLIC
        imgui_setup
        implot_setup
        tinyfiledialogs
        opencv_core
        opencv_imgproc
        opencv_imgcodecs
        opencv_videoio
        opencv_highgui
        spdlog::spdlog
        ixwebsocket
        stb_image_setup
        tag
        serial_setup
        nlohmann_json::nlohmann_json
        Microsoft.GSL::GSL
        fmt::fmt
        magic_enum::magic_enum
        unordered_dense::unordered_dense
        stdx
    )

    target_include_directories(mbrdaq_core_${TARGET_NAME} PUBLIC include)
    target_include_directories(mbrdaq_core_${TARGET_NAME} SYSTEM PUBLIC
        ${CMAKE_BINARY_DIR}
        ${CMAKE_SOURCE_DIR}/vendor/opencv/include
        ${CMAKE_SOURCE_DIR}/vendor/opencv/modules/core/include
        ${CMAKE_SOURCE_DIR}/vendor/opencv/modules/imgproc/include
        ${CMAKE_SOURCE_DIR}/vendor/opencv/modules/imgcodecs/include
        ${CMAKE_SOURCE_DIR}/vendor/opencv/modules/videoio/include
        ${CMAKE_SOURCE_DIR}/vendor/opencv/modules/highgui/include
    )

    target_compile_options(mbrdaq_core_${TARGET_NAME} PRIVATE ${BASE_FLAGS})
    target_compile_options(mbrdaq_core_${TARGET_NAME} PRIVATE ${EXTRA_FLAGS})
    if(APPLE)
        target_link_options(mbrdaq_core_${TARGET_NAME} PRIVATE ${EXTRA_FLAGS} "-static-libstdc++")
    elseif(WIN32)
        target_link_options(mbrdaq_core_${TARGET_NAME} PRIVATE ${EXTRA_FLAGS})
    endif()
    set_target_properties(mbrdaq_core_${TARGET_NAME}
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

    add_executable(mbrdaq_tests_${TARGET_NAME} ${TEST_SOURCES})
    target_link_libraries(mbrdaq_tests_${TARGET_NAME}
        PRIVATE catch2_${TARGET_NAME} mbrdaq_core_${TARGET_NAME})
    target_include_directories(mbrdaq_tests_${TARGET_NAME}
        PRIVATE include tests)
    target_compile_options(mbrdaq_tests_${TARGET_NAME}
        PRIVATE ${WARNING_IGNORE})

    target_compile_options(mbrdaq_tests_${TARGET_NAME}
        PRIVATE ${EXTRA_FLAGS})
    target_link_options(mbrdaq_tests_${TARGET_NAME} PRIVATE ${EXTRA_FLAGS})
    set_target_properties(mbrdaq_tests_${TARGET_NAME}
        PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${OUT_DIR})
endfunction()

function(create_standard_target EXTRA_FLAGS TARGET_NAME)
    create_mbrdaq_suite("${TARGET_NAME}"
        "${EXTRA_FLAGS}"
        "${CMAKE_BINARY_DIR}/${TARGET_NAME}")

    if(WIN32 AND NOT TARGET_NAME STREQUAL "debug")
        add_executable(mbrdaq_${TARGET_NAME} WIN32 src/main.cpp)
        if (NOT MINGW)
            target_link_options(mbrdaq_${TARGET_NAME} PRIVATE "LINKER:/ENTRY:mainCRTStartup")
        endif()
    else()
        add_executable(mbrdaq_${TARGET_NAME} src/main.cpp)
    endif()

    target_link_libraries(mbrdaq_${TARGET_NAME} PRIVATE mbrdaq_core_${TARGET_NAME})
    add_custom_target(${TARGET_NAME} DEPENDS
        mbrdaq_tests_${TARGET_NAME} mbrdaq_${TARGET_NAME})

    set_target_properties(mbrdaq_${TARGET_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${TARGET_NAME}
        OUTPUT_NAME ${PROJECT_NAME})

    # Windows still depends on a custom ffmpeg dll provided by opencv
    if(WIN32)
        add_custom_command(TARGET mbrdaq_${TARGET_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${CMAKE_BINARY_DIR}/bin"
                "$<TARGET_FILE_DIR:mbrdaq_${TARGET_NAME}>"
            COMMENT "Copy FFMPEG to output directory"
        )
    endif()
endfunction()
