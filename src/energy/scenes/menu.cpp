// SPDX-FileCopyrightText: 2026 Juan Medina
// SPDX-License-Identifier: MIT

#include "menu.hpp"

#include <memory>
#include <optional>
#include <pxe/app.hpp>
#include <pxe/components/button.hpp>
#include <pxe/components/component.hpp>
#include <pxe/components/label.hpp>
#include <pxe/result.hpp>
#include <pxe/scenes/scene.hpp>
#include <spdlog/spdlog.h>

namespace energy {

auto menu::init(pxe::app &app) -> pxe::result<> {
	if(const auto err = scene::init(app).unwrap(); err) {
		return pxe::error("failed to initialize base component", *err);
	}

	SPDLOG_INFO("menu scene initialized");

	if(const auto err = play_button_.init(app).unwrap(); err) {
		return pxe::error("failed to initialize play button component", *err);
	}

	play_button_.set_text("Play!");
	play_button_.set_position({.x = 0, .y = 0});
	play_button_.set_size({.width = 80, .height = 35});
	play_button_.set_font_size(large_font_size);

	button_click_ = app.bind_event<pxe::button::click>(this, &menu::on_button_click);

	if(const auto err = title_.init(app).unwrap(); err) {
		return pxe::error("failed to initialize title label", *err);
	}

	title_.set_text("Energy Swap");
	title_.set_font_size(60);

	return true;
}

auto menu::end() -> pxe::result<> {
	if(const auto err = play_button_.end().unwrap(); err) {
		return pxe::error("failed to end play button component", *err);
	}

	if(const auto err = title_.end().unwrap(); err) {
		return pxe::error("failed to end logo texture", *err);
	}

	get_app().unsubscribe(button_click_);
	return scene::end();
}

auto menu::update(const float delta) -> pxe::result<> {
	if(const auto err = play_button_.update(delta).unwrap(); err) {
		return pxe::error("failed to update play button component", *err);
	}
	return true;
}

auto menu::draw() -> pxe::result<> {
	if(const auto err = play_button_.draw().unwrap(); err) {
		return pxe::error("failed to draw play button component", *err);
	}

	if(const auto err = title_.draw().unwrap(); err) {
		return pxe::error("failed to draw logo texture", *err);
	}

	return true;
}

auto menu::layout(const pxe::size screen_size) -> pxe::result<> {
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
	return true;
}

auto menu::enable() -> pxe::result<> {
	if(const auto err = scene::enable().unwrap(); err) {
		return pxe::error("failed to enable base scene", *err);
	}

	if(const auto err = get_app().play_music("resources/music/menu.ogg", 0.5F).unwrap(); err) {
		return pxe::error(menu_music_path, *err);
	}

	return true;
}

auto menu::on_button_click(const pxe::button::click &evt) -> pxe::result<> {
	if(evt.id == play_button_.get_id()) {
		get_app().post_event(go_to_game{});
	}
	return true;
}

} // namespace energy