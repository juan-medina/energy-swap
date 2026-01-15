// SPDX-FileCopyrightText: 2026 Juan Medina
// SPDX-License-Identifier: MIT

#include "menu.hpp"

#include <pxe/app.hpp>
#include <pxe/components/button.hpp>
#include <pxe/components/component.hpp>
#include <pxe/components/label.hpp>
#include <pxe/result.hpp>
#include <pxe/scenes/scene.hpp>

#include <memory>
#include <optional>
#include <spdlog/spdlog.h>

namespace energy {

auto menu::init(pxe::app &app) -> pxe::result<> {
	if(const auto err = scene::init(app).unwrap(); err) {
		return pxe::error("failed to initialize base component", *err);
	}

	SPDLOG_INFO("menu scene initialized");

	if(const auto err = register_component<pxe::button>().unwrap(play_button_); err) {
		return pxe::error("failed to register play button component", *err);
	}

	std::shared_ptr<pxe::button> play_button;
	if(const auto err = get_component<pxe::button>(play_button_).unwrap(play_button); err) {
		return pxe::error("failed to get play button component", *err);
	}

	play_button->set_text("Play!");
	play_button->set_position({.x = 0, .y = 0});
	play_button->set_size({.width = 80, .height = 35});
	play_button->set_font_size(large_font_size);

	button_click_ = app.bind_event<pxe::button::click>(this, &menu::on_button_click);

	if(const auto err = register_component<pxe::label>().unwrap(title_); err) {
		return pxe::error("failed to register title label", *err);
	}

	std::shared_ptr<pxe::label> title;
	if(const auto err = get_component<pxe::label>(title_).unwrap(title); err) {
		return pxe::error("failed to get title label component", *err);
	}

	title->set_text("Energy Swap");
	title->set_font_size(60);

	return true;
}

auto menu::end() -> pxe::result<> {
	get_app().unsubscribe(button_click_);
	return scene::end();
}

auto menu::layout(const pxe::size screen_size) -> pxe::result<> {
	std::shared_ptr<pxe::label> title;
	if(const auto err = get_component<pxe::label>(title_).unwrap(title); err) {
		return pxe::error("failed to get title label component", *err);
	}

	const auto [label_width, label_height] = title->get_size();
	title->set_position({
		.x = (screen_size.width - label_width) / 2.0F,
		.y = (screen_size.height * 0.2F) - (label_height / 2.0F),
	});

	std::shared_ptr<pxe::button> play_button;
	if(const auto err = get_component<pxe::button>(play_button_).unwrap(play_button); err) {
		return pxe::error("failed to get play button component", *err);
	}

	const auto [button_width, button_height] = play_button->get_size();
	play_button->set_position({
		.x = (screen_size.width - button_width) / 2.0F,
		.y = (screen_size.height - button_height) / 2.0F,
	});

	return true;
}

auto menu::show() -> pxe::result<> {
	if(const auto err = scene::show().unwrap(); err) {
		return pxe::error("failed to enable base scene", *err);
	}

	if(const auto err = get_app().play_music("resources/music/menu.ogg", 0.5F).unwrap(); err) {
		return pxe::error(menu_music_path, *err);
	}

	return true;
}

auto menu::on_button_click(const pxe::button::click &evt) -> pxe::result<> {
	if(evt.id == play_button_) {
		get_app().post_event(go_to_game{});
	}
	return true;
}

} // namespace energy