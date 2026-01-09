# SPDX-FileCopyrightText: 2026 Juan Medina
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.13)

set(APP_NAME "energy-swap")

project(${APP_NAME}
        VERSION 0.1.0.0
        DESCRIPTION "A minimalist puzzle game about organizing colored energy units"
        LANGUAGES CXX
)

# C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Generate compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Collect sources recursively
file(GLOB_RECURSE APP_HEADER_FILES "src/*.hpp")
file(GLOB_RECURSE APP_SOURCE_FILES "src/*.cpp")


# Create executable
add_executable(${APP_NAME} ${APP_HEADER_FILES} ${APP_SOURCE_FILES})

# Compiler warnings (target-specific)
if (MSVC)
    target_compile_options(${APP_NAME} PRIVATE
            /W4
            /WX
            /permissive-
    )
else ()
    target_compile_options(${APP_NAME} PRIVATE
            -Wall
            -Wextra
            -pedantic
            -Werror
    )
endif ()

# Detect Emscripten
set(EMSCRIPTEN FALSE)
if (CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
    set(EMSCRIPTEN TRUE)
endif ()


# Add spdlog
add_subdirectory(external/spdlog)
target_link_libraries(${APP_NAME} PRIVATE spdlog::spdlog_header_only)

# Set spdlog to trace level, we can filter later in runtime
target_compile_definitions(${APP_NAME} PRIVATE
        SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_TRACE
)

# Add raylib
add_subdirectory(external/raylib)
target_link_libraries(${APP_NAME} PRIVATE raylib)
target_include_directories(${APP_NAME} SYSTEM PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/external/raylib/src)

# Add raygui
target_include_directories(${APP_NAME} SYSTEM PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/external/raygui/src)

# Add jsoncons
target_include_directories(${APP_NAME} SYSTEM PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/external/jsoncons/include)


# No supported on Emscripten
if (NOT EMSCRIPTEN)
    # Add Boxer
    add_subdirectory(external/boxer)
    target_link_libraries(${APP_NAME} PRIVATE Boxer)
endif ()

# Emscripten specific
if (EMSCRIPTEN)
    # Produce an .html file as the "executable"
    set(CMAKE_EXECUTABLE_SUFFIX ".html")

    set(CUSTOM_SHELL ${CMAKE_SOURCE_DIR}/src/web/template.html)

    # Reasonable defaults; tweak later if needed
    target_link_options(${APP_NAME} PRIVATE
            "--shell-file=${CUSTOM_SHELL}"
            "--preload-file=${CMAKE_SOURCE_DIR}/resources@resources"
            "-sUSE_GLFW=3"
            "-sASYNCIFY"
            "-sALLOW_MEMORY_GROWTH=1"
            "--bind"
            "-sEXPORTED_RUNTIME_METHODS=['HEAPF32','HEAP32','HEAPU8']"
    )

    # Rename output to index.html
    set_target_properties(${APP_NAME} PROPERTIES OUTPUT_NAME "index")

    # Set favicon
    configure_file(${CMAKE_SOURCE_DIR}/resources/icon/icon.ico favicon.ico COPYONLY)

endif ()

# Windows specific
if (WIN32)
    # fix raylib issues on Windows
    target_compile_definitions(${APP_NAME} PRIVATE
            WIN32_LEAN_AND_MEAN
            NOMINMAX
            NOUSER
            NOGDI
    )

    set(APP_ICON_RESOURCE "${CMAKE_SOURCE_DIR}/src/res/energy-swap.rc")
    target_sources(${APP_NAME} PRIVATE ${APP_ICON_RESOURCE})

    configure_file(${CMAKE_SOURCE_DIR}/resources/icon/icon.ico icon.ico COPYONLY)

endif ()

# At top-level, before you need Python
find_package(Python3 COMPONENTS Interpreter REQUIRED)

set(VERSION_JSON ${CMAKE_SOURCE_DIR}/resources/version/version.json)
set(VERSION_SCRIPT ${CMAKE_SOURCE_DIR}/scripts/version.py)

# Always bump version when the target is built
add_custom_command(
        TARGET ${APP_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "Incrementing build number..."
        COMMAND ${Python3_EXECUTABLE} ${VERSION_SCRIPT} ${VERSION_JSON}
        COMMENT "Version bumped"
)

# Only copy resources if they changed (native only)
if (NOT EMSCRIPTEN)
    add_custom_command(
            TARGET ${APP_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E echo "Syncing resources if needed..."
            COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
            ${CMAKE_SOURCE_DIR}/resources
            $<TARGET_FILE_DIR:${APP_NAME}>/resources
            COMMENT "Copying resources only if changed"
    )
endif ()
