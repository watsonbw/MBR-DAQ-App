function(create_mbrdaq_suite TARGET_NAME EXTRA_FLAGS OUT_DIR)
    add_library(mbrdaq_core_${TARGET_NAME} STATIC ${LIB_SOURCES})
    set_target_properties(mbrdaq_core_${TARGET_NAME} PROPERTIES
        AUTOMOC ON
        AUTOUIC OFF
        AUTORCC OFF
    )

    target_link_libraries(mbrdaq_core_${TARGET_NAME} PUBLIC
        tinyfiledialogs
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
        Qt6::Core
        Qt6::Widgets
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
        target_link_options(mbrdaq_core_${TARGET_NAME} PRIVATE ${EXTRA_FLAGS})
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

    # Test Executable
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
    elseif(APPLE)
        add_executable(mbrdaq_${TARGET_NAME} MACOSX_BUNDLE src/main.cpp)
        set_target_properties(mbrdaq_${TARGET_NAME} PROPERTIES
            MACOSX_BUNDLE_BUNDLE_NAME ${PROJECT_NAME}
            MACOSX_BUNDLE_GUI_IDENTIFIER "com.mbr.daq.${TARGET_NAME}"
            MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
            MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION}
        )
    else()
        add_executable(mbrdaq_${TARGET_NAME} src/main.cpp)
    endif()

    target_link_libraries(mbrdaq_${TARGET_NAME} PRIVATE mbrdaq_core_${TARGET_NAME})

    add_custom_target(${TARGET_NAME} DEPENDS
        mbrdaq_tests_${TARGET_NAME} mbrdaq_${TARGET_NAME})

    set_target_properties(mbrdaq_${TARGET_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${TARGET_NAME}
        OUTPUT_NAME ${PROJECT_NAME})

    if(WIN32)
        # Ensure destination directory exists
        add_custom_command(TARGET mbrdaq_${TARGET_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/bin"
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${CMAKE_BINARY_DIR}/bin"
                "$<TARGET_FILE_DIR:mbrdaq_${TARGET_NAME}>"
            COMMENT "Copy FFMPEG binaries to output directory"
        )

        # Automatically copy MSYS2 GCC/MinGW compiler runtimes
        add_custom_command(TARGET mbrdaq_${TARGET_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "C:/msys64/ucrt64/bin/libstdc++-6.dll"
                "C:/msys64/ucrt64/bin/libgcc_s_seh-1.dll"
                "C:/msys64/ucrt64/bin/libwinpthread-1.dll"
                "$<TARGET_FILE_DIR:mbrdaq_${TARGET_NAME}>"
            COMMENT "Copying MinGW runtime DLLs"
        )

        if(WINDEPLOYQT_EXE)
            add_custom_command(TARGET mbrdaq_${TARGET_NAME} POST_BUILD
                COMMAND ${WINDEPLOYQT_EXE}
                    --dir "$<TARGET_FILE_DIR:mbrdaq_${TARGET_NAME}>"
                    --no-translations
                    "$<TARGET_FILE:mbrdaq_${TARGET_NAME}>"
                COMMENT "Deploy Qt runtime libraries"
            )
        endif()
    elseif(APPLE)
        if(MACDEPLOYQT_EXE)
            add_custom_command(TARGET mbrdaq_${TARGET_NAME} POST_BUILD
                COMMAND ${MACDEPLOYQT_EXE}
                    "$<TARGET_BUNDLE_DIR:mbrdaq_${TARGET_NAME}>"
                COMMENT "Deploy Qt frameworks into app bundle"
            )
        endif()
    endif()
endfunction()