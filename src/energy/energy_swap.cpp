// SPDX-FileCopyrightText: 2026 Juan Medina
// SPDX-License-Identifier: MIT

#include "energy_swap.hpp"

#include <pxe/app.hpp>
#include <pxe/main.hpp>
#include <pxe/result.hpp>
#include <pxe/scenes/scene.hpp>

#include "scenes/cosmic.hpp"
#include "scenes/game.hpp"
#include "scenes/level_selection.hpp"
#include "scenes/mode.hpp"

#include <cstddef>
#include <optional>
#include <string>

PXE_MAIN(energy::energy_swap)

namespace energy {

auto energy_swap::init() -> pxe::result<> {
	if(const auto err = app::init().unwrap(); err) {
		return pxe::error{"failed to initialize base app", *err};
	}

	set_clear_color(clear_color);

	if(const auto err = load_sprite_sheet(sprite_sheet_name, sprite_sheet_path).unwrap(); err) {
		return pxe::error("failed to initialize sprite sheet", *err);
	}

	set_logo(sprite_sheet_name, logo_frame);
	if(const auto err = load_sfx(battery_click_sfx, battery_click_sfx_path).unwrap(); err) {
		return pxe::error{"failed to load battery click sfx", *err};
	}

	if(const auto err = load_sfx(zap_sfx, zap_sfx_path).unwrap(); err) {
		return pxe::error{"failed to load zap sfx", *err};
	}

	if(const auto err = level_manager_.load_levels().unwrap(); err) {
		return pxe::error{"failed to load levels", *err};
	}

	// Load current level from settings (defaults to max reached level or 1)
	level_manager_.set_max_reached_level(static_cast<size_t>(get_setting<int>(max_level_key, 1)));
	level_manager_.set_current_level(level_manager_.get_max_reached_level());

	level_selection_scene_ = register_scene<level_selection>(false);
	game_scene_ = register_scene<game>(false);
	mode_scene_ = register_scene<mode>(false);
	cosmic_scene_ = register_scene<cosmic>(false);

	set_main_scene(mode_scene_);

	// subscribe to events
	next_level_ = on_event<game::next_level>(this, &energy_swap::on_next_level);
	game_back_ = on_event<game::back>(this, &energy_swap::on_game_back);
	reset_ = on_event<game::reset_level>(this, &energy_swap::on_reset_level);
	level_selected_ = bind_event<level_selected>(this, &energy_swap::on_level_selected);
	back_from_level_selection_ = on_event<level_selection::back>(this, &energy_swap::on_back_from_level_selection);
	back_from_mode_ = on_event<mode::back>(this, &energy_swap::on_back_from_mode);
	mode_selected_ = bind_event<mode::selected>(this, &energy_swap::on_mode_selected);
	back_from_cosmic_ = on_event<cosmic::back>(this, &energy_swap::on_back_from_cosmic);
	difficulty_selected_ = bind_event<cosmic::selected>(this, &energy_swap::on_difficulty_selected);

	return true;
}

auto energy_swap::end() -> pxe::result<> {
	// unsubscribe from events
	unsubscribe(next_level_);
	unsubscribe(game_back_);
	unsubscribe(reset_);
	unsubscribe(level_selected_);
	unsubscribe(back_from_level_selection_);
	unsubscribe(back_from_mode_);
	unsubscribe(mode_selected_);
	unsubscribe(back_from_cosmic_);

	// unload sfx
	if(const auto err = unload_sfx(battery_click_sfx).unwrap(); err) {
		return pxe::error{"failed to unload battery click sfx", *err};
	}

	if(const auto err = unload_sprite_sheet(sprite_sheet_name).unwrap(); err) {
		return pxe::error("failed to end sprite sheet", *err);
	}

	return app::end();
}

auto energy_swap::on_next_level() -> pxe::result<> {
	level_manager_.set_current_level(level_manager_.get_current_level() + 1);

	// Update max reached level if we've progressed further
	if(level_manager_.get_current_level() > level_manager_.get_max_reached_level()) {
		level_manager_.set_max_reached_level(level_manager_.get_current_level());
		set_setting(max_level_key, static_cast<int>(level_manager_.get_current_level()));
		if(const auto err = save_settings().unwrap(); err) {
			return pxe::error("failed to save settings", *err);
		}
	}

	return reset(game_scene_);
}

auto energy_swap::on_game_back() -> pxe::result<> {
	post_event(back_to_menu_from{.id = game_scene_});

	return true;
}

auto energy_swap::on_reset_level() -> pxe::result<> {
	return reset(game_scene_);
}

auto energy_swap::on_level_selected(const level_selected &evt) -> pxe::result<> {
	level_manager_.set_current_level(evt.level);
	return replace_scene(level_selection_scene_, game_scene_);
}

auto energy_swap::on_back_from_level_selection() -> pxe::result<> {
	return replace_scene(level_selection_scene_, mode_scene_);
}

auto energy_swap::on_back_from_mode() -> pxe::result<> {
	post_event(back_to_menu_from{.id = mode_scene_});

	return true;
}

auto energy_swap::on_mode_selected(const mode::selected &evt) -> pxe::result<> {
	switch(evt.mode) {
	case level_manager::mode::classic:
		level_manager_.set_current_level(level_manager_.get_max_reached_level());
		level_manager_.set_mode(evt.mode);
		return replace_scene(mode_scene_, level_selection_scene_);
	case level_manager::mode::cosmic:
		level_manager_.set_mode(evt.mode);
		return replace_scene(mode_scene_, cosmic_scene_);
	default:
		return pxe::error("unknown mode selected");
	}
}

auto energy_swap::on_back_from_cosmic() -> pxe::result<> {
	return replace_scene(cosmic_scene_, mode_scene_);
}

auto energy_swap::on_difficulty_selected(const cosmic::selected &evt) -> pxe::result<> {
	level_manager_.set_mode(level_manager::mode::cosmic);
	level_manager_.set_difficulty(evt.difficulty);
	level_manager_.set_current_level(1);
	return replace_scene(cosmic_scene_, game_scene_);
}

} // namespace energy
