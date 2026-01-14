# SPDX-FileCopyrightText: 2026 Juan Medina
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.13)

set(APP_NAME "energy-swap")

project(${APP_NAME}
        VERSION 0.1.0.0
        DESCRIPTION "A minimalist puzzle game about organizing colored energy units"
        LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Add pxe submodule and helper
add_subdirectory(external/pxe)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/external/pxe/cmake")
include(pxe_game)

# Define the game target
pxe_add_game(${APP_NAME})