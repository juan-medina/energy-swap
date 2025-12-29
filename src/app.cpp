// SPDX-FileCopyrightText: 2025 Juan Medina
// SPDX-License-Identifier: MIT

#include "app.hpp"

#include <raylib.h>
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

    DrawText("Hello, world!", 1900 / 2 - MeasureText("Hello, world!", 20) / 2, 1080 / 2 - 10, 20, RAYWHITE);

    EndDrawing();
}
void energy::app::setup_log() {
    spdlog::set_pattern(empty_format);
    SPDLOG_INFO(banner);

    spdlog::set_pattern(color_line_format);
    SetTraceLogCallback([](int logLevel, const char *text, va_list args) {
        char buffer[1024];
        vsnprintf(buffer, sizeof(buffer), text, args);
        spdlog::level::level_enum level;
        switch(logLevel) {
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
            level = spdlog::level::info;
            break;
        }
        spdlog::log(level, "[raylib] {}", buffer);
    });
}