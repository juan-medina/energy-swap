// SPDX-FileCopyrightText: 2026 Juan Medina
// SPDX-License-Identifier: MIT

#include "energy_swap.hpp"

#include "scenes/game.hpp"
#include "scenes/license.hpp"
#include "scenes/menu.hpp"

#include <fstream>

namespace energy {

auto energy_swap::init() -> engine::result<> {
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

	if(const auto err = load_sound(battery_click_sound, battery_click_sound_path).ko(); err) {
		return engine::error{"failed to load battery click sound", *err};
	}

	if(const auto err = load_levels().ko(); err) {
		return engine::error{"failed to load levels", *err};
	}

	license_scene_ = register_scene<license>();
	menu_scene_ = register_scene<menu>(false);
	game_scene_ = register_scene<game>(false);

	license_accepted_ = on_event<license::accepted>(this, &energy_swap::on_license_accepted);
	go_to_game_ = on_event<menu::go_to_game>(this, &energy_swap::on_go_to_game);
	next_level_ = on_event<game::next_level>(this, &energy_swap::on_next_level);
	game_back_ = on_event<game::back>(this, &energy_swap::on_game_back);

	return true;
}

auto energy_swap::end() -> engine::result<> {
	unsubscribe(license_accepted_);
	unsubscribe(go_to_game_);
	unsubscribe(next_level_);
	unsubscribe(game_back_);

	if(const auto err = unload_sound(click_sound).ko(); err) {
		return engine::error{"failed to unload click sound", *err};
	}

	if(const auto err = unload_sound(battery_click_sound).ko(); err) {
		return engine::error{"failed to unload battery click sound", *err};
	}

	return app::end();
}

auto energy_swap::on_license_accepted() -> engine::result<> {
	auto err = disable_scene(license_scene_).ko();
	if(err) {
		return engine::error("fail to disable license scene", *err);
	}

	if(err = enable_scene(menu_scene_).ko(); err) {
		return engine::error("fail to enable menu scene", *err);
	}

	return true;
}

auto energy_swap::on_go_to_game() -> engine::result<> {
	current_level_ = 1;
	auto err = disable_scene(menu_scene_).ko();
	if(err) {
		return engine::error("fail to disable menu scene", *err);
	}

	if(err = enable_scene(game_scene_).ko(); err) {
		return engine::error("fail to enable game scene", *err);
	}

	return true;
}

auto energy_swap::load_levels() -> engine::result<> {
	if(std::ifstream const level_file(levels_path); !level_file.is_open()) {
		return engine::error(std::format("can not load levels file: {}", levels_path));
	}

	char *text = nullptr;
	if(text = LoadFileText(levels_path); text == nullptr) {
		return engine::error(std::format("failed to load levels file from {}", levels_path));
	}

	std::istringstream stream(text);
	std::string line;
	while(std::getline(stream, line)) {
		levels_.emplace_back(line);
	}

	UnloadFileText(text);

	return true;
}

auto energy_swap::on_next_level() -> engine::result<> {
	current_level_++;
	if(const auto err = re_enable_scene(game_scene_).ko(); err) {
		return engine::error("fail to re-enable game scene", *err);
	}
	return true;
}

auto energy_swap::on_game_back() -> engine::result<> {
	auto err = disable_scene(game_scene_).ko();
	if(err) {
		return engine::error("fail to disable game scene", *err);
	}

	if(err = enable_scene(menu_scene_).ko(); err) {
		return engine::error("fail to enable menu scene", *err);
	}

	return true;
}

} // namespace energy
