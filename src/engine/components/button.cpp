// SPDX-FileCopyrightText: 2026 Juan Medina
// SPDX-License-Identifier: MIT

#include "button.hpp"

#include "../app.hpp"
#include "../result.hpp"
#include "component.hpp"
#include "raylib.h"
#include "ui_component.hpp"

#include <optional>
#include <raygui.h>

namespace engine {
int button::next_id = 0;

button::button(): id_(++next_id) {}

auto button::init(app &app) -> result<> {
	if(const auto err = ui_component::init(app).unwrap(); err) {
		return error("failed to initialize base UI component", *err);
	}

	return true;
}

auto button::end() -> result<> {
	return ui_component::end();
}

auto button::update(const float delta) -> result<> {
	if(const auto err = ui_component::update(delta).unwrap(); err) {
		return error("failed to update base UI component", *err);
	}

	return true;
}

auto button::draw() -> result<> {
	if(const auto err = ui_component::draw().unwrap(); err) {
		return error("failed to draw base UI component", *err);
	}

	if(!is_visible()) {
		return true;
	}

	const auto [x, y] = get_position();
	const auto [width, height] = get_size();

	GuiSetFont(get_font());
	GuiSetStyle(DEFAULT, TEXT_SIZE, static_cast<int>(get_font_size()));

	if(const Rectangle rect{.x = x, .y = y, .width = width, .height = height}; GuiButton(rect, text_.c_str())) {
		if(const auto err = play_click_sound().unwrap(); err) {
			return error("failed to play click sound", *err);
		}
		get_app().post_event(click{.id = id_});
	}

	return true;
}

} // namespace engine