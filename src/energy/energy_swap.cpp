// SPDX-FileCopyrightText: 2026 Juan Medina
// SPDX-License-Identifier: MIT

#include "energy_swap.hpp"

#include <pxe/app.hpp>
#include <pxe/main.hpp>
#include <pxe/result.hpp>
#include <pxe/scenes/scene.hpp>

#include "scenes/game.hpp"
#include "scenes/level_selection.hpp"

#include <raylib.h>

#include <cstddef>
#include <format>
#include <fstream>
#include <optional>
#include <sstream>
#include <string>

PXE_MAIN(energy::energy_swap)

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

	// Load current level from settings (defaults to max reached level or 1)
	current_level_ = get_max_reached_level();

	level_selection_scene_ = register_scene<level_selection>(false);
	game_scene_ = register_scene<game>(false);

	set_main_scene(level_selection_scene_);

	// subscribe to events
	next_level_ = on_event<game::next_level>(this, &energy_swap::on_next_level);
	game_back_ = on_event<game::back>(this, &energy_swap::on_game_back);
	reset_ = on_event<game::reset_level>(this, &energy_swap::on_reset_level);
	level_selected_ = bind_event<level_selected>(this, &energy_swap::on_level_selected);

	return true;
}

auto energy_swap::end() -> pxe::result<> {
	// unsubscribe from events
	unsubscribe(next_level_);
	unsubscribe(game_back_);
	unsubscribe(reset_);
	unsubscribe(level_selected_);

	// unload sfx
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

	// Update max reached level if we've progressed further
	if(const auto max_reached = get_setting<int>("max_reached_level", 1);
	   current_level_ > static_cast<size_t>(max_reached)) {
		set_setting("max_reached_level", static_cast<int>(current_level_));
		if(const auto err = save_settings().unwrap(); err) {
			return pxe::error("failed to save settings", *err);
		}
	}

	if(const auto err = reset_scene(game_scene_).unwrap(); err) {
		return pxe::error("fail to reset game scene", *err);
	}
	return true;
}

auto energy_swap::on_game_back() -> pxe::result<> {
	if(const auto err = hide_scene(game_scene_).unwrap(); err) {
		return pxe::error("fail to hide game scene", *err);
	}

	post_event(back_to_menu{});

	return true;
}

auto energy_swap::on_reset_level() -> pxe::result<> {
	if(const auto err = reset_scene(game_scene_).unwrap(); err) {
		return pxe::error("fail to reset game scene", *err);
	}
	return true;
}

auto energy_swap::on_level_selected(const level_selected &evt) -> pxe::result<> {
	current_level_ = evt.level;

	if(const auto err = hide_scene(level_selection_scene_).unwrap(); err) {
		return pxe::error("fail to hide level selection scene", *err);
	}

	if(const auto err = show_scene(game_scene_).unwrap(); err) {
		return pxe::error("fail to show game scene", *err);
	}

	return true;
}

} // namespace energy
