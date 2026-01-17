// SPDX-FileCopyrightText: 2026 Juan Medina
// SPDX-License-Identifier: MIT

#include "energy_swap.hpp"

#include <pxe/app.hpp>
#include <pxe/result.hpp>
#include <pxe/scenes/scene.hpp>

#include "scenes/game.hpp"

#include <raylib.h>

#include <format>
#include <fstream>
#include <optional>
#include <sstream>
#include <string>

namespace energy {

auto energy_swap::init() -> pxe::result<> {
	if(const auto err = app::init().unwrap(); err) {
		return pxe::error{"failed to initialize base app", *err};
	}

	set_clear_color(clear_color);

	if(const auto err = set_default_font(font_path).unwrap(); err) {
		return pxe::error{"failed to set default font", *err};
	}

	if(const auto err = load_sfx(click_sfx, click_sfx_path).unwrap(); err) {
		return pxe::error{"failed to load button sfx", *err};
	}

	if(const auto err = load_sfx(battery_click_sfx, battery_click_sfx_path).unwrap(); err) {
		return pxe::error{"failed to load battery click sfx", *err};
	}

	if(const auto err = load_sfx(zap_sfx, zap_sfx_path).unwrap(); err) {
		return pxe::error{"failed to load zap sfx", *err};
	}

	if(const auto err = load_levels().unwrap(); err) {
		return pxe::error{"failed to load levels", *err};
	}

	game_scene_ = register_scene<game>(false);
	set_main_scene(game_scene_);
	next_level_ = on_event<game::next_level>(this, &energy_swap::on_next_level);
	game_back_ = on_event<game::back>(this, &energy_swap::on_game_back);
	reset_ = on_event<game::reset_level>(this, &energy_swap::on_reset_level);

	return true;
}

auto energy_swap::end() -> pxe::result<> {
	unsubscribe(next_level_);
	unsubscribe(game_back_);
	unsubscribe(reset_);

	if(const auto err = unload_sfx(click_sfx).unwrap(); err) {
		return pxe::error{"failed to unload click sfx", *err};
	}

	if(const auto err = unload_sfx(battery_click_sfx).unwrap(); err) {
		return pxe::error{"failed to unload battery click sfx", *err};
	}

	return app::end();
}

auto energy_swap::load_levels() -> pxe::result<> {
	if(std::ifstream const level_file(levels_path); !level_file.is_open()) {
		return pxe::error(std::format("can not load levels file: {}", levels_path));
	}

	char *text = nullptr;
	if(text = LoadFileText(levels_path); text == nullptr) {
		return pxe::error(std::format("failed to load levels file from {}", levels_path));
	}

	std::istringstream stream(text);
	std::string line;
	while(std::getline(stream, line)) {
		levels_.emplace_back(line);
	}

	UnloadFileText(text);

	return true;
}

auto energy_swap::on_next_level() -> pxe::result<> {
	current_level_++;
	if(const auto err = reset_scene(game_scene_).unwrap(); err) {
		return pxe::error("fail to reset game scene", *err);
	}
	return true;
}

auto energy_swap::on_game_back() -> pxe::result<> {
	if(const auto err = hide_scene(game_scene_).unwrap()) {
		return pxe::error("fail to disable game scene", *err);
	}
	current_level_ = 1;

	post_event(back_to_menu{});

	return true;
}

auto energy_swap::on_reset_level() -> pxe::result<> {
	if(const auto err = reset_scene(game_scene_).unwrap(); err) {
		return pxe::error("fail to reset game scene", *err);
	}
	return true;
}

} // namespace energy
