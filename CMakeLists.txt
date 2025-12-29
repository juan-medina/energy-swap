# SPDX-FileCopyrightText: 2025 Juan Medina
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 4.0)

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

# Collect sources
file(GLOB APP_HEADER_FILES "src/*.h")
file(GLOB APP_SOURCE_FILES "src/*.cpp")

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

# Add raylib
add_subdirectory(external/raylib)
target_link_libraries(${APP_NAME} PRIVATE raylib)

# Add raygui
target_include_directories(${APP_NAME} SYSTEM PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/external/raygui/src)

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

    # Reasonable defaults; tweak later if needed
    target_link_options(${APP_NAME} PRIVATE
            "-sUSE_GLFW=3"
            "-sASYNCIFY"
            "-sALLOW_MEMORY_GROWTH=1"
    )
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
endif ()
