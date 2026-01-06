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

	play_button_.set_text("Play!");
	play_button_.set_position({.x = 0, .y = 0});
	play_button_.set_size({.width = 80, .height = 35});
	play_button_.set_font_size(large_font_size);

	button_click_ = app.bind_event<engine::button::click>(this, &menu::on_button_click);

	if(const auto err = title_.init(app).ko(); err) {
		return engine::error("failed to initialize title label", *err);
	}

	title_.set_text("Energy Swap");
	title_.set_font_size(60);

	return true;
}

auto menu::end() -> engine::result<> {
	if(const auto err = play_button_.end().ko(); err) {
		return engine::error("failed to end play button component", *err);
	}

	if(const auto err = title_.end().ko(); err) {
		return engine::error("failed to end logo texture", *err);
	}

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

	if(const auto err = title_.draw().ko(); err) {
		return engine::error("failed to draw logo texture", *err);
	}

	return true;
}

auto menu::layout(const engine::size screen_size) -> void {
	const auto [label_width, label_height] = title_.get_size();
	title_.set_position({
		.x = (screen_size.width - label_width) / 2.0F,
		.y = (screen_size.height * 0.2F) - (label_height / 2.0F),
	});

	const auto [button_width, button_height] = play_button_.get_size();
	play_button_.set_position({
		.x = (screen_size.width - button_width) / 2.0F,
		.y = (screen_size.height - button_height) / 2.0F,
	});
}

auto menu::on_button_click(const engine::button::click &evt) -> void {
	if(evt.id == play_button_.get_id()) {
		get_app().post_event(go_to_game{});
	}
}

} // namespace energy