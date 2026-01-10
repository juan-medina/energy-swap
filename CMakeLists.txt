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

# Detect Emscripten
set(EMSCRIPTEN FALSE)
if (CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
    set(EMSCRIPTEN TRUE)
endif ()

# Python (needed for version bump / rc generation)
find_package(Python3 COMPONENTS Interpreter REQUIRED)

set(VERSION_JSON ${CMAKE_SOURCE_DIR}/resources/version/version.json)
set(VERSION_SCRIPT ${CMAKE_SOURCE_DIR}/scripts/version.py)

# --- Version bump (all platforms, BEFORE compilation) ---
# This runs at build time, not configure time.
add_custom_target(version_bump
        COMMAND ${CMAKE_COMMAND} -E echo "Incrementing build number..."
        COMMAND ${Python3_EXECUTABLE} ${VERSION_SCRIPT} ${VERSION_JSON}
        BYPRODUCTS ${VERSION_JSON}
        COMMENT "Version bumped"
        VERBATIM
)

# --- Windows resource generation (ONLY Windows) ---
if (WIN32)
    enable_language(RC)

    # fix raylib issues on Windows
    # (definitions are applied later after target creation)

    set(APP_ICON_ICO ${CMAKE_SOURCE_DIR}/src/res/icon.ico)
    set(APP_RC_OUT ${CMAKE_BINARY_DIR}/energy-swap.rc)

    # Generate .rc from version.json AFTER bump, BEFORE compile/link
    add_custom_command(
            OUTPUT ${APP_RC_OUT}
            COMMAND ${CMAKE_COMMAND} -E echo "Generating Windows version resource (.rc)..."
            COMMAND ${Python3_EXECUTABLE} ${VERSION_SCRIPT}
            --emit-rc
            ${VERSION_JSON}
            ${APP_RC_OUT}
            ${APP_ICON_ICO}
            DEPENDS version_bump ${VERSION_JSON} ${VERSION_SCRIPT} ${APP_ICON_ICO}
            VERBATIM
    )
endif ()

# --- Create executable (include generated .rc on Windows) ---
if (WIN32)
    add_executable(${APP_NAME}
            ${APP_HEADER_FILES}
            ${APP_SOURCE_FILES}
            ${APP_RC_OUT}
    )
else ()
    add_executable(${APP_NAME}
            ${APP_HEADER_FILES}
            ${APP_SOURCE_FILES}
    )
endif ()

# Ensure bump happens before building the executable (all platforms)
add_dependencies(${APP_NAME} version_bump)

# If Windows, make sure .rc is treated as RC and is visible in IDE
if (WIN32)
    set_source_files_properties(${APP_RC_OUT} PROPERTIES LANGUAGE RC)
    source_group("Resources" FILES ${APP_RC_OUT})

    target_compile_definitions(${APP_NAME} PRIVATE
            WIN32_LEAN_AND_MEAN
            NOMINMAX
            NOUSER
            NOGDI
    )
endif ()

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

# Add spdlog
add_subdirectory(external/spdlog)
target_link_libraries(${APP_NAME} PRIVATE spdlog::spdlog_header_only)

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
    add_subdirectory(external/boxer)
    target_link_libraries(${APP_NAME} PRIVATE Boxer)
endif ()

# Emscripten specific
if (EMSCRIPTEN)
    set(CMAKE_EXECUTABLE_SUFFIX ".html")

    set(CUSTOM_SHELL ${CMAKE_SOURCE_DIR}/src/web/template.html)

    target_link_options(${APP_NAME} PRIVATE
            "--shell-file=${CUSTOM_SHELL}"
            "--preload-file=${CMAKE_SOURCE_DIR}/resources@resources"
            "-sUSE_GLFW=3"
            "-sASYNCIFY"
            "-sALLOW_MEMORY_GROWTH=1"
            "--bind"
            "-sEXPORTED_RUNTIME_METHODS=['HEAPF32','HEAP32','HEAPU8']"
    )

    set_target_properties(${APP_NAME} PROPERTIES OUTPUT_NAME "index")

    configure_file(${CMAKE_SOURCE_DIR}/src/res/icon.ico favicon.ico COPYONLY)
endif ()

# Only copy resources if they changed (native only)
if (NOT EMSCRIPTEN)
    add_custom_command(
            TARGET ${APP_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E echo "Syncing resources if needed..."
            COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
            ${CMAKE_SOURCE_DIR}/resources
            $<TARGET_FILE_DIR:${APP_NAME}>/resources
            COMMENT "Copying resources only if changed"
            VERBATIM
    )
endif ()
