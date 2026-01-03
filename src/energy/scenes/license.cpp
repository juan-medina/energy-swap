// SPDX-FileCopyrightText: 2026 Juan Medina
// SPDX-License-Identifier: MIT

#include "license.hpp"

#include "../../engine/app.hpp"

#include <spdlog/spdlog.h>

namespace energy {

auto license::init(engine::app &app) -> engine::result<> {
    app_ = app;

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

    if(const auto err = accept_button_.init(app).ko(); err) {
        return engine::error("Failed to initialize accept button component", *err);
    }

    accept_button_.set_text("Accept");
    accept_button_.set_position({.x = 0, .y = 0});
    accept_button_.set_size({.width = 100, .height = 40});

    click_ = app.subscribe<engine::button::click>([this](const engine::button::click &evt) -> void {
        if(evt.id == accept_button_.get_id()) {
            on_license_accepted();
        }
    });

    return true;
}

auto license::end() -> engine::result<> {
    app_->get().unsubscribe(click_);
    app_.reset();
    return true;
}

auto license::update(float /*delta*/) -> engine::result<> {
    return true;
}

auto license::draw() -> engine::result<> {
    if(const auto err = scroll_text_.draw().ko(); err) {
        return engine::error("Failed to draw scroll text component", *err);
    }
    if(const auto err = accept_button_.draw().ko(); err) {
        return engine::error("Failed to draw accept button", *err);
    }
    return true;
}

auto license::layout(const Vector2 screen_size) -> void {
    const auto min_width = screen_size.x * 2.5F / 3.0F;
    scroll_text_.set_size({.width = std::min(min_width, 1200.0F), .height = screen_size.y * 4.0F / 5.0F});

    scroll_text_.set_position({.x = (screen_size.x - scroll_text_.get_size().width) / 2.0F,
                               .y = (screen_size.y - scroll_text_.get_size().height) / 2.0F});

    const auto [width, height] = accept_button_.get_size();
    float const button_x = (screen_size.x - width) / 2.0F;
    float const button_y = scroll_text_.get_pos().y + scroll_text_.get_size().height + 10;
    accept_button_.set_position({.x = button_x, .y = button_y});
}
auto license::on_license_accepted() -> void {
    SPDLOG_INFO("License accepted by user");
}

} // namespace energy