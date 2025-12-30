// SPDX-FileCopyrightText: 2025 Juan Medina
// SPDX-License-Identifier: MIT

#include "version_display.hpp"

#include "spdlog/spdlog.h"

#include <algorithm>
#include <cassert>

#ifdef _WIN32
#    include <shellapi.h>
#    include <windows.h>
#elif defined(__APPLE__) || defined(__linux__)
#    include <unistd.h>
#elif defined(__EMSCRIPTEN__)
#    include <emscripten/emscripten.h>
#    include <emscripten/val.h>
#endif

auto energy::version_display::init(const version &version) -> result<> {
    version_ = version;
    static_assert(components_colors.size() == 8);
    assert(components_colors.size() == parts_.size());
    parts_ = {
        part{.text = std::string("v"), .color = components_colors.at(0), .offset = 0.0F},
        part{.text = std::to_string(version_.major), .color = components_colors.at(1), .offset = 0.0F},
        part{.text = std::string("."), .color = components_colors.at(2), .offset = 0.0F},
        part{.text = std::to_string(version_.minor), .color = components_colors.at(3), .offset = 0.0F},
        part{.text = std::string("."), .color = components_colors.at(4), .offset = 0.0F},
        part{.text = std::to_string(version.patch), .color = components_colors.at(5), .offset = 0.0F},
        part{.text = std::string("."), .color = components_colors.at(6), .offset = 0.0F},
        part{.text = std::to_string(version.build), .color = components_colors.at(7), .offset = 0.0F},
    };

    // Calculate total width
    bounds_.width = 0;
    bounds_.height = 0;

    font_ = GetFontDefault();

    for(auto &part: parts_) {
        const auto [size_x, size_y] = MeasureTextEx(font_, part.text.c_str(), font_size, 1.0F);
        part.offset = bounds_.width;
        bounds_.width += size_x + parts_spacing;
        bounds_.height = std::max(size_y, bounds_.height);
    }
    return true;
}

auto energy::version_display::end() -> result<> {
    version_ = {};
    parts_ = {};
    return true;
}

auto energy::version_display::layout(const Vector2 screen_size) -> void {
    screen_size_ = screen_size;
    bounds_.x = screen_size_.x - bounds_.width - margin;
    bounds_.y = screen_size_.y - bounds_.height - margin;
}

auto energy::version_display::update(float /*delta*/) -> result<> {
    // check for mouse click on version display
    if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        const auto [x, y] = GetMousePosition();
        if(x >= bounds_.x && x <= bounds_.x + bounds_.width && y >= bounds_.y && y <= bounds_.y + bounds_.height) {
            if(const auto err = open_url("https://juan-medina.com").ko(); err) {
                return error("Failed updating version display", *err);
            }
        }
    }
    return true;
}

auto energy::version_display::draw() const -> void {
    Vector2 pos = {.x = bounds_.x, .y = bounds_.y};
    for(const auto &part: parts_) {
        pos.x = bounds_.x + part.offset;
        DrawTextEx(font_, part.text.c_str(), pos, font_size, 1.0F, part.color);
    }
}

#include <vector>

auto energy::version_display::open_url(const std::string &url) -> result<> {
#ifdef _WIN32
    if(auto *result = ShellExecuteA(nullptr, "open", url.c_str(), nullptr, nullptr, 1);
       reinterpret_cast<intptr_t>(result) <= 32) { // NOLINT(*-pro-type-reinterpret-cast)
        return error("Failed to open URL using shell execute");
    }
    return true;
#elif defined(__APPLE__) || defined(__linux__)
#    ifdef __APPLE__
    const std::string open_command = "open";
#    else
    const std::string open_command = "xdg-open";
#    endif
    const auto pid = fork();
    if(pid == 0) {
        std::vector cmd(open_command.begin(), open_command.end());
        cmd.push_back('\0');
        std::vector arg(url.begin(), url.end());
        arg.push_back('\0');
        const std::vector<char *> argv{cmd.data(), arg.data(), nullptr};
        execvp(cmd.data(), argv.data());
        _exit(1);
    }
    if(pid > 0) {
        return true;
    }
    return error("Failed to fork process to open URL");
#elif defined(__EMSCRIPTEN__)
    using emscripten::val;

    const auto document = val::global("document");
    auto anchor = document.call<val>("createElement", val("a"));
    anchor.set("href", url);
    anchor.set("target", "_blank");
    anchor.set("rel", "noopener noreferrer");
    const auto body_list = document.call<val>("getElementsByTagName", val("body"));
    const auto body = body_list.call<val>("item", val(0));

    body.call<void>("appendChild", anchor);
    anchor.call<void>("click");
    body.call<void>("removeChild", anchor);

    return true;
#endif
}