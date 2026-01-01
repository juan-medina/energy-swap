// SPDX-FileCopyrightText: 2025 Juan Medina
// SPDX-License-Identifier: MIT

#include "app.hpp"

#include "scenes/game_overlay.hpp"

#include <cstdarg>
#include <fstream>
#include <jsoncons/json.hpp>
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

auto engine::app::init() -> result<> {
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
    InitWindow(1920, 1080, title_.c_str());
    SetTargetFPS(60);

    // init components
    SPDLOG_INFO("Initializing");

    register_scene(0, std::make_unique<game_overlay>(), 999, true);

    // init scenes
    for(auto &scene_info: scenes_) {
        if(err = scene_info.scene_ptr->init(*this).ko(); err) {
            return error(std::format("Failed to initialize scene with id {}", scene_info.id), *err);
        }
    }

    SPDLOG_INFO("Application started");
    return true;
}

auto engine::app::run() -> result<> {
    if(const auto err = init().ko(); err) {
        return error("error running the application", *err);
    }

    while(!WindowShouldClose()) {
        if(const auto err = update().ko(); err) {
            return error("error updating the application", *err);
        }
        if(const auto err = internal_draw().ko(); err) {
            return error("error drawing the application", *err);
        }
    }

    SPDLOG_INFO("Application ended");
    return true;
}

auto engine::app::update() -> result<> {
    if(Vector2 const screen_size = {.x = static_cast<float>(GetScreenWidth()),
                                    .y = static_cast<float>(GetScreenHeight())};
       screen_size_.x != screen_size.x || screen_size_.y != screen_size.y) {
        screen_size_ = screen_size;
        SPDLOG_INFO("Display resized to {}x{}", static_cast<int>(screen_size_.x), static_cast<int>(screen_size_.y));

        // screen size changed, tell scenes to layout
        for(const auto &scene_info: scenes_) {
            scene_info.scene_ptr->layout(screen_size_);
        }
    }

    // update scenes
    for(auto &scene_info: scenes_) {
        if(const auto err = scene_info.scene_ptr->update(GetFrameTime()).ko(); err) {
            return error(std::format("Failed to initialize scene with id {}", scene_info.id), *err);
        }
    }

    return true;
}

auto engine::app::draw() const -> result<> {
    // draw scenes
    for(const auto &scene_info: scenes_) {
        if(const auto err = scene_info.scene_ptr->draw().ko(); err) {
            return error(std::format("Failed to initialize scene with id {}", scene_info.id), *err);
        }
    }
    return true;
}

auto engine::app::setup_log() -> result<> {
    spdlog::set_pattern(empty_format);
    const auto version_str = std::format("{}.{}.{}.{}", version_.major, version_.minor, version_.patch, version_.build);
    SPDLOG_INFO(std::vformat(banner, std::make_format_args(version_str)));

    spdlog::set_pattern(color_line_format);
    SetTraceLogCallback(log_callback);

#ifdef NDEBUG
    spdlog::set_level(spdlog::level::err);
    SetTraceLogLevel(LOG_ERROR);
#else
    spdlog::set_level(spdlog::level::debug);
    SetTraceLogLevel(LOG_DEBUG);
#endif
    return true;
}

void engine::app::log_callback(const int log_level, const char *text, va_list args) {
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
auto engine::app::internal_draw() const -> result<> {
    BeginDrawing();
    ClearBackground(Color{.r = 20, .g = 49, .b = 59, .a = 255});

    if(const auto err = draw().ko(); err) {
        return error("error during internal draw", *err);
    }
    EndDrawing();
    return true;
}

auto engine::app::parse_version(const std::string &path) -> result<version> {
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
