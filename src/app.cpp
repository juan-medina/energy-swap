// SPDX-FileCopyrightText: 2025 Juan Medina
// SPDX-License-Identifier: MIT

#include "app.hpp"

#include <raylib.h>
#define RAYGUI_IMPLEMENTATION
#define RAYGUI_USE_RAYLIB
#include <cstdarg>
#include <raygui.h>
#include <spdlog/spdlog.h>

static const auto banner = R"(
  ______                               _____
 |  ____|                             / ____|
 | |__   _ __   ___ _ __ __ _ _   _  | (_____      ____ _ _ __
 |  __| | '_ \ / _ \ '__/ _` | | | |  \___ \ \ /\ / / _` | '_ \
 | |____| | | |  __/ | | (_| | |_| |  ____) \ V  V / (_| | |_) |
 |______|_| |_|\___|_|  \__, |\__, | |_____/ \_/\_/ \__,_| .__/
                         __/ | __/ |                     | |
                        |___/ |___/                      |_|    )";

static const auto empty_format = "%v";
static const auto color_line_format = "[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v %@";

auto energy::app::run() -> result<> {
    setup_log();

    SPDLOG_INFO("Starting application");

    InitWindow(1920, 1080, "testing raylib");
    SetTargetFPS(60);

    SPDLOG_INFO("Application started");

    while(!WindowShouldClose()) {
        update();
        draw();
    }

    SPDLOG_INFO("Application ended");
    return true;
}

void energy::app::update() {}

void energy::app::draw() {
    BeginDrawing();
    ClearBackground(SKYBLUE);

    // set large text size for button
    GuiSetStyle(DEFAULT, TEXT_SIZE, 32);

    // Get current screen size
    const int screen_width = GetScreenWidth();
    const int screen_height = GetScreenHeight();

    // Button size
    constexpr int button_width = 200;
    constexpr int button_height = 60;
    const int start_x = (screen_width / 2) - (button_width / 2);
    const int start_y = (screen_height / 2) - (button_height / 2);

    // Centered button rectangle
    const Rectangle button_bounds = {.x = static_cast<float>(start_x),
                                     .y = static_cast<float>(start_y),
                                     .width = static_cast<float>(button_width),
                                     .height = static_cast<float>(button_height)};

    // Draw the button; returns true when clicked this frame
    if(GuiButton(button_bounds, "Click me") != 0) {
        SPDLOG_INFO("button clicked");
    }

    EndDrawing();
}

void energy::app::setup_log() {
    spdlog::set_pattern(empty_format);
    SPDLOG_INFO(banner);

    spdlog::set_pattern(color_line_format);
    SetTraceLogCallback(log_callback);
}

void energy::app::log_callback(const int log_level, const char *text, va_list args) {
    constexpr std::size_t initial_size = 1024;

    // One buffer per thread, reused across calls
    thread_local std::vector<char> buffer(initial_size);

    va_list args_copy{};      // NOLINT(*-pro-type-vararg)
    va_copy(args_copy, args); // NOLINT(*-pro-bounds-array-to-pointer-decay)
    int const needed =
        std::vsnprintf(buffer.data(), buffer.size(), text, args_copy); // NOLINT(*-pro-bounds-array-to-pointer-decay)
    va_end(args_copy);                                                 // NOLINT(*-pro-bounds-array-to-pointer-decay)

    if(needed < 0) {
        SPDLOG_ERROR("[raylib] log formatting error in log callback");
        return;
    }

    if(static_cast<std::size_t>(needed) >= buffer.size()) {
        // Grow once (or very rarely)
        buffer.resize(static_cast<std::size_t>(needed) + 1);
        std::vsnprintf(buffer.data(), buffer.size(), text, args);
    }

    spdlog::level::level_enum level = spdlog::level::info;
    switch(log_level) {
    case LOG_TRACE:
        level = spdlog::level::trace;
        break;
    case LOG_DEBUG:
        level = spdlog::level::debug;
        break;
    case LOG_INFO:
        level = spdlog::level::info;
        break;
    case LOG_WARNING:
        level = spdlog::level::warn;
        break;
    case LOG_ERROR:
        level = spdlog::level::err;
        break;
    case LOG_FATAL:
        level = spdlog::level::critical;
        break;
    default:
        break;
    }

    spdlog::log(level, "[raylib] {}", buffer.data());
}