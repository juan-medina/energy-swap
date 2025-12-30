// SPDX-FileCopyrightText: 2025 Juan Medina
// SPDX-License-Identifier: MIT

#include "app.hpp"

#include <cstdarg>
#include <fstream>
#include <jsoncons/json.hpp>
#include <raylib.h>
#define RAYGUI_IMPLEMENTATION
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
                        |___/ |___/                      |_| v{})";

static const auto empty_format = "%v";
static const auto color_line_format = "[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v %@";

auto energy::app::init() -> result<> {
    auto [version, err] = parse_version(version_file_path).ok();
    if(err) {
        return error("error parsing the version", *err);
    }
    version_ = *version;

    if(err = setup_log().ko(); err) {
        return error("error initializing the application", *err);
    }

    SPDLOG_INFO("Starting application");

#ifdef PLATFORM_DESKTOP
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
#endif
    InitWindow(1920, 1080, "testing raylib");
    SetTargetFPS(60);

    SPDLOG_INFO("Application started");
    return true;
}

auto energy::app::run() -> result<> {
    if(const auto err = init().ko(); err) {
        return error("error running the application", *err);
    }

    while(!WindowShouldClose()) {
        update();
        draw();
    }

    SPDLOG_INFO("Application ended");
    return true;
}

void energy::app::update() {}

void energy::app::draw() const {
    BeginDrawing();
    ClearBackground(Color{.r = 20, .g = 49, .b = 59, .a = 255});

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

    draw_version();

    EndDrawing();
}

auto energy::app::setup_log() -> result<> {
    spdlog::set_pattern(empty_format);
    const auto version_str = std::format("{}.{}.{}.{}", version_.major, version_.minor, version_.patch, version_.build);
    SPDLOG_INFO(std::vformat(banner, std::make_format_args(version_str)));

    spdlog::set_pattern(color_line_format);
    SetTraceLogCallback(log_callback);
    return true;
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

auto energy::app::parse_version(const std::string &path) -> result<version> {
    std::ifstream const file(path);
    if(!file.is_open()) {
        return error(std::format("Version file not found: {}", path));
    }

    std::stringstream buffer;
    buffer << file.rdbuf();

    std::error_code error_code;
    jsoncons::json_decoder<jsoncons::json> decoder;
    jsoncons::json_stream_reader reader(buffer, decoder);
    reader.read(error_code);

    if(error_code) {
        return error(std::format("JSON parse error: {}", error_code.message()));
    }

    const auto &parser = decoder.get_result();

    // NOLINTNEXTLINE(*-pro-bounds-avoid-unchecked-container-access)
    if(!parser.contains("version") || !parser["version"].is_object()) {
        return error("Failed to parse version JSON: [\"version\"] field missing or not an object");
    }

    const auto &object = parser["version"]; // NOLINT(*-pro-bounds-avoid-unchecked-container-access)
    return version{.major = object.get_value_or<int>("major", 0),
                   .minor = object.get_value_or<int>("minor", 0),
                   .patch = object.get_value_or<int>("patch", 0),
                   .build = object.get_value_or<int>("build", 0)};
}

void energy::app::draw_version() const {
    const std::array part_strs = {std::string("v"),
                                  std::to_string(version_.major),
                                  std::to_string(version_.minor),
                                  std::to_string(version_.patch),
                                  std::to_string(version_.build)};

    constexpr std::array colors = {
        Color{.r = 0xF0, .g = 0x00, .b = 0xF0, .a = 0xFF}, // #F000F0 (v)
        Color{.r = 0xFF, .g = 0x00, .b = 0x00, .a = 0xFF}, // #FF0000 (major)
        Color{.r = 0xFF, .g = 0xA5, .b = 0x00, .a = 0xFF}, // #FFA500 (minor)
        Color{.r = 0xFF, .g = 0xFF, .b = 0x00, .a = 0xFF}, // #FFFF00 (patch)
        Color{.r = 0x00, .g = 0xFF, .b = 0x00, .a = 0xFF}  // #00FF00 (build)
    };

    assert(part_strs.size() == colors.size());

    constexpr auto dot_color = WHITE;
    constexpr int font_size = 20;
    constexpr int margin = 10;
    constexpr int dot_spacing = 4;

    // we silence the linter because for video games we want branch prediction and cache usage,
    //   and we know these arrays are constant size so we prefer using indices

    // Calculate total width
    int total_width = 0;
    for(std::size_t i = 0; i < colors.size(); ++i) {
        // NOLINTNEXTLINE(*-pro-bounds-constant-array-index, *-pro-bounds-avoid-unchecked-container-access)
        total_width += MeasureText(part_strs[i].c_str(), font_size);
        if(i < colors.size() - 1) {
            total_width += (2 * dot_spacing) + MeasureText(".", font_size);
        }
    }

    int pos_x = GetScreenWidth() - total_width - margin;
    const int pos_y = GetScreenHeight() - font_size - margin;

    for(std::size_t i = 0; i < colors.size(); ++i) {
        // NOLINTNEXTLINE(*-pro-bounds-constant-array-index, *-pro-bounds-avoid-unchecked-container-access)
        DrawText(part_strs[i].c_str(), pos_x, pos_y, font_size, colors[i]);
        // NOLINTNEXTLINE(*-pro-bounds-constant-array-index, *-pro-bounds-avoid-unchecked-container-access)
        pos_x += MeasureText(part_strs[i].c_str(), font_size);
        if(i < 4) {
            pos_x += dot_spacing;
            DrawText(".", pos_x, pos_y, font_size, dot_color);
            pos_x += MeasureText(".", font_size) + dot_spacing;
        }
    }
}
