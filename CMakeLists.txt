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

file(GLOB_RECURSE APP_HEADER_FILES "src/*.hpp")
file(GLOB_RECURSE APP_SOURCE_FILES "src/*.cpp")

set(EMSCRIPTEN FALSE)
if (CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
    set(EMSCRIPTEN TRUE)
endif ()

find_package(Python3 COMPONENTS Interpreter REQUIRED)

set(VERSION_JSON ${CMAKE_SOURCE_DIR}/resources/version/version.json)
set(VERSION_SCRIPT ${CMAKE_SOURCE_DIR}/scripts/version.py)

# --- Version bump toggle ---
option(ENABLE_VERSION_BUMP "Increment build number at build time" ON)

# --- Version bump (optional, all platforms, build-time) ---
if (ENABLE_VERSION_BUMP)
    add_custom_target(version_bump
            COMMAND ${CMAKE_COMMAND} -E echo "Incrementing build number..."
            COMMAND ${Python3_EXECUTABLE} ${VERSION_SCRIPT} ${VERSION_JSON}
            BYPRODUCTS ${VERSION_JSON}
            COMMENT "Version bumped"
            VERBATIM
    )
endif ()

# --- Windows resource generation (ONLY Windows) ---
if (WIN32)
    enable_language(RC)

    set(APP_ICON_ICO ${CMAKE_SOURCE_DIR}/src/res/icon.ico)
    set(APP_RC_OUT ${CMAKE_BINARY_DIR}/energy-swap.rc)

    # Make .rc depend on bumped json only when bumping is enabled
    if (ENABLE_VERSION_BUMP)
        set(_RC_DEPENDS version_bump ${VERSION_JSON} ${VERSION_SCRIPT} ${APP_ICON_ICO})
    else ()
        set(_RC_DEPENDS ${VERSION_JSON} ${VERSION_SCRIPT} ${APP_ICON_ICO})
    endif ()

    add_custom_command(
            OUTPUT ${APP_RC_OUT}
            COMMAND ${CMAKE_COMMAND} -E echo "Generating Windows version resource (.rc)..."
            COMMAND ${Python3_EXECUTABLE} ${VERSION_SCRIPT}
            --emit-rc
            ${VERSION_JSON}
            ${APP_RC_OUT}
            ${APP_ICON_ICO}
            DEPENDS ${_RC_DEPENDS}
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

    target_link_options(${APP_NAME} PRIVATE
            $<$<CONFIG:Debug>:/SUBSYSTEM:CONSOLE>
            $<$<NOT:$<CONFIG:Debug>>:/SUBSYSTEM:WINDOWS>
    )
else ()
    add_executable(${APP_NAME}
            ${APP_HEADER_FILES}
            ${APP_SOURCE_FILES}
    )
endif ()

# Ensure bump happens before building the executable (only if enabled)
if (ENABLE_VERSION_BUMP)
    add_dependencies(${APP_NAME} version_bump)
endif ()

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

if (MSVC)
    target_compile_options(${APP_NAME} PRIVATE /W4 /WX /permissive-)
else ()
    target_compile_options(${APP_NAME} PRIVATE -Wall -Wextra -pedantic -Werror)
endif ()

add_subdirectory(external/pxe)
target_link_libraries(${APP_NAME} PRIVATE pxe)


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
            "-sEXPORTED_RUNTIME_METHODS=['HEAPF32','HEAP32','HEAPU8','requestFullscreen']"
    )

    set_target_properties(${APP_NAME} PROPERTIES OUTPUT_NAME "index")
    configure_file(${CMAKE_SOURCE_DIR}/src/res/icon.ico favicon.ico COPYONLY)
endif ()

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
