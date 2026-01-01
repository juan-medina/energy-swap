// SPDX-FileCopyrightText: 2025 Juan Medina
// SPDX-License-Identifier: MIT

#include "license.hpp"

#include <raygui.h>
#include <spdlog/spdlog.h>

namespace energy {

auto license::init(engine::app & /*app*/) -> engine::result<> {
    return true;
}

auto license::end() -> engine::result<> {
    return true;
}

auto license::update(float /*delta*/) -> engine::result<> {
    return true;
}

auto license::draw() const -> engine::result<> {
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

    return true;
}

auto license::layout(Vector2 /*screen_size*/) -> void {}

} // namespace energy