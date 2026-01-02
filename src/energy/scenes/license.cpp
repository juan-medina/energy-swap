// SPDX-FileCopyrightText: 2026 Juan Medina
// SPDX-License-Identifier: MIT

#include "license.hpp"

#include "../../engine/app.hpp"

#include <raygui.h>
#include <spdlog/spdlog.h>

namespace energy {

auto license::init(engine::app &app) -> engine::result<> {
    SPDLOG_INFO("License scene initialized");

    if(const auto err = scroll_text_.init(app).ko(); err) {
        return engine::error("Failed to initialize scroll text component", *err);
    }

    char *text = nullptr;
    if(text = LoadFileText(license_path); text == nullptr) {
        return engine::error(std::format("Failed to load license file from {}", license_path));
    }

    scroll_text_.set_text(text);
    UnloadFileText(text);
    scroll_text_.set_position({.x = 10, .y = 10});
    scroll_text_.set_size({.width = 500, .height = 400});
    scroll_text_.set_title("License");
    return true;
}

auto license::end() -> engine::result<> {
    return true;
}

auto license::update(float /*delta*/) -> engine::result<> {
    return true;
}

auto license::draw() -> engine::result<> {
    if(const auto err = scroll_text_.draw().ko(); err) {
        return engine::error("Failed to draw scroll text component", *err);
    }
    GuiSetStyle(DEFAULT, TEXT_SIZE, font_size);
    if(GuiButton(button_bounds_, "Accept") != 0) {
        SPDLOG_INFO("License accepted...");
    }

    return true;
}

auto license::layout(const Vector2 screen_size) -> void {
    const auto min_width = screen_size.x * 2.5F / 3.0F;
    scroll_text_.set_size({.width = std::min(min_width, 1200.0F), .height = screen_size.y * 4.0F / 5.0F});

    scroll_text_.set_position({.x = (screen_size.x - scroll_text_.get_size().width) / 2.0F,
                               .y = (screen_size.y - scroll_text_.get_size().height) / 2.0F});

    button_bounds_ = {.x = (screen_size.x - 100) / 2.0F,
                      .y = scroll_text_.get_pos().y + scroll_text_.get_size().height + 10,
                      .width = 100,
                      .height = 40};
}

} // namespace energy