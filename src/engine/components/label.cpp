// SPDX-FileCopyrightText: 2026 Juan Medina
// SPDX-License-Identifier: MIT

#include "label.hpp"

#include "../app.hpp"

#include <raygui.h>

auto engine::label::init(app &app) -> result<> {
	if(const auto err = ui_component::init(app).ko(); err) {
		return error("failed to initialize base UI component", *err);
	}

	return true;
}

auto engine::label::end() -> result<> {
	return ui_component::end();
}

auto engine::label::update(float delta) -> result<> {
	if(const auto err = ui_component::update(delta).ko(); err) {
		return error("failed to update base UI component", *err);
	}

	return true;
}

auto engine::label::draw() -> result<> {
	if(const auto err = ui_component::draw().ko(); err) {
		return error("failed to draw base UI component", *err);
	}

	if(!is_visible()) {
		return true;
	}

	const auto [x, y] = get_pos();
	const auto [width, height] = get_size();

	GuiSetFont(get_font());
	const auto default_text_color = GuiGetStyle(DEFAULT, TEXT_COLOR_NORMAL);

	GuiSetStyle(DEFAULT, TEXT_SIZE, static_cast<int>(get_font_size()));
	GuiSetStyle(DEFAULT, TEXT_COLOR_NORMAL, ColorToInt(WHITE));

	GuiLabel({.x = x, .y = y, .width = width, .height = height}, text_.c_str());

	GuiSetStyle(DEFAULT, TEXT_COLOR_NORMAL, default_text_color);

	return true;
}

auto engine::label::set_text(const std::string &text) -> void {
	text_ = text;
	calculate_size();
}
auto engine::label::set_font_size(const float &size) -> void {
	ui_component::set_font_size(size);
	calculate_size();
}

auto engine::label::calculate_size() -> void {
	const auto [x, y] = MeasureTextEx(get_font(), text_.c_str(), get_font_size(), 1.0F);
	set_size({.width = x, .height = y});
}
