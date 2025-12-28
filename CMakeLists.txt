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
add_executable(${APP_NAME}
        ${APP_HEADER_FILES}
        ${APP_SOURCE_FILES}
)

# Compiler warnings (target-specific)
if (MSVC)
    target_compile_options(${APP_NAME} PRIVATE
            /W4
            /WX
            /permissive-
    )
else()
    target_compile_options(${APP_NAME} PRIVATE
            -Wall
            -Wextra
            -pedantic
            -Werror
    )
endif()
