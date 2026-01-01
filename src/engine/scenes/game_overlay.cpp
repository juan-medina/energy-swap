// SPDX-FileCopyrightText: 2025 Juan Medina
// SPDX-License-Identifier: MIT

#include "game_overlay.hpp"

namespace engine {

auto game_overlay::init(app &app) -> result<> {
    if(const auto err = version_display_.init(app).ko(); err) {
        return error("Failed to initialize version display", *err);
    }
    return true;
}

auto game_overlay::end() -> result<> {
    return true;
}

auto game_overlay::update(float delta) -> result<> {
    if(const auto err = version_display_.update(delta).ko(); err) {
        return error("Failed to update version display", *err);
    }
    return true;
}

auto game_overlay::draw() const -> result<> {
    if(const auto err = version_display_.draw().ko(); err) {
        return error("Failed to draw version display", *err);
    }
    return true;
}

auto game_overlay::layout(const Vector2 screen_size) -> void {
    // position version display at bottom-right corner with margin
    const auto [width, height] = version_display_.get_size();
    version_display_.set_position({
        .x = screen_size.x - width - margin,
        .y = screen_size.y - height - margin,
    });
}

} // namespace engine