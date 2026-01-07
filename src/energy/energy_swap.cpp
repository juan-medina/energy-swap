// SPDX-FileCopyrightText: 2026 Juan Medina
// SPDX-License-Identifier: MIT

#include "energy_swap.hpp"

#include "scenes/game.hpp"
#include "scenes/license.hpp"
#include "scenes/menu.hpp"

auto energy::energy_swap::init() -> engine::result<> {
	if(const auto err = app::init().ko(); err) {
		return engine::error{"failed to initialize base app", *err};
	}

	set_clear_color(clear_color);

	if(const auto err = set_default_font(font_path).ko(); err) {
		return engine::error{"failed to set default font", *err};
	}

	if(const auto err = load_sound(click_sound, click_sound_path).ko(); err) {
		return engine::error{"failed to load button sound", *err};
	}

	license_scene_ = register_scene<license>();
	menu_scene_ = register_scene<menu>(false);
	game_scene_ = register_scene<game>(false);

	license_accepted_ = on_event<license::accepted>(this, &energy_swap::on_license_accepted);
	go_to_game_ = on_event<menu::go_to_game>(this, &energy_swap::on_go_to_game);

	return true;
}

auto energy::energy_swap::end() -> engine::result<> {
	unsubscribe(license_accepted_);
	return app::end();
}

auto energy::energy_swap::on_license_accepted() -> engine::result<> {
	auto err = disable_scene(license_scene_).ko();
	if(err) {
		return engine::error("fail to disable license scene", *err);
	}

	if(err = enable_scene(menu_scene_).ko(); err) {
		return engine::error("fail to enable menu scene", *err);
	}

	if(err = play_music("resources/music/menu.ogg", 0.5F).ko(); err) {
		return engine::error("fail to play menu music", *err);
	}

	return true;
}

auto energy::energy_swap::on_go_to_game() -> engine::result<> {
	auto err = disable_scene(menu_scene_).ko();
	if(err) {
		return engine::error("fail to disable menu scene", *err);
	}

	if(err = enable_scene(game_scene_).ko(); err) {
		return engine::error("fail to enable game scene", *err);
	}

	return true;
}
