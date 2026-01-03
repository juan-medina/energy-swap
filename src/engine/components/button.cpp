// SPDX-FileCopyrightText: 2026 Juan Medina
// SPDX-License-Identifier: MIT

#include "button.hpp"

#include "../app.hpp"

#include <raygui.h>

int engine::button::next_id = 0;

engine::button::button(): id_(++next_id) {}

auto engine::button::init(app &app) -> result<> {
    if(const auto err = ui_component::init(app).ko(); err) {
        return error("failed to initialize base UI component", *err);
    }

    return true;
}

auto engine::button::end() -> result<> {
    return ui_component::end();
}

auto engine::button::update(float delta) -> result<> {
    if(const auto err = ui_component::update(delta).ko(); err) {
        return error("failed to update base UI component", *err);
    }

    return true;
}

auto engine::button::draw() -> result<> {
    if(const auto err = ui_component::draw().ko(); err) {
        return error("failed to draw base UI component", *err);
    }

    const auto [x, y] = get_pos();
    const auto [width, height] = get_size();

    GuiSetFont(get_font());
    GuiSetStyle(DEFAULT, TEXT_SIZE, static_cast<int>(get_font_size()));

    if(const Rectangle rect{.x = x, .y = y, .width = width, .height = height}; GuiButton(rect, text_.c_str())) {
        if(const auto err = play_click_sound().ko(); err) {
            return error("failed to play click sound", *err);
        }
        get_app().post_event(click{.id = id_});
    }

    return true;
}