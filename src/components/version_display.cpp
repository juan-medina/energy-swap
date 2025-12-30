// SPDX-FileCopyrightText: 2025 Juan Medina
// SPDX-License-Identifier: MIT

#include "version_display.hpp"

#include <algorithm>
#include <cassert>

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

auto energy::version_display::update(float /*delta*/) -> void {}

auto energy::version_display::draw() const -> void {
    Vector2 pos = {.x = bounds_.x, .y = bounds_.y};
    for(const auto &part: parts_) {
        pos.x = bounds_.x + part.offset;
        DrawTextEx(font_, part.text.c_str(), pos, font_size, 1.0F, part.color);
    }
}