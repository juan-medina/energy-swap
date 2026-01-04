// SPDX-FileCopyrightText: 2026 Juan Medina
// SPDX-License-Identifier: MIT

#include "menu.hpp"

#include "../../engine/app.hpp"

#include <spdlog/spdlog.h>

namespace energy {

auto menu::init(engine::app &app) -> engine::result<> {
	if(const auto err = scene::init(app).ko(); err) {
		return engine::error("failed to initialize base component", *err);
	}

	SPDLOG_INFO("menu scene initialized");

	if(const auto err = play_button_.init(app).ko(); err) {
		return engine::error("failed to initialize play button component", *err);
	}

	play_button_.set_text("Play");
	play_button_.set_position({.x = 0, .y = 0});
	play_button_.set_size({.width = 200, .height = 60});
	play_button_.set_font_size(large_font_size);

	button_click_ = app.bind_event<engine::button::click>(this, &menu::on_button_click);

	return true;
}

auto menu::end() -> engine::result<> {
	get_app().unsubscribe(button_click_);
	return scene::end();
}

auto menu::update(const float delta) -> engine::result<> {
	if(const auto err = play_button_.update(delta).ko(); err) {
		return engine::error("failed to update play button component", *err);
	}
	return true;
}

auto menu::draw() -> engine::result<> {
	if(const auto err = play_button_.draw().ko(); err) {
		return engine::error("failed to draw play button component", *err);
	}
	return true;
}

auto menu::layout(const Vector2 screen_size) -> void {
	const auto [width, height] = play_button_.get_size();
	const float button_x = (screen_size.x - width) / 2.0F;
	const float button_y = (screen_size.y - height) / 2.0F;
	play_button_.set_position({.x = button_x, .y = button_y});
}

auto menu::on_button_click(const engine::button::click &evt) const -> void {
	if(evt.id == play_button_.get_id()) {
		SPDLOG_INFO("play button clicked");
	}
}

} // namespace energy