// SPDX-FileCopyrightText: 2026 Juan Medina
// SPDX-License-Identifier: MIT

#pragma once

#include <pxe/app.hpp>
#include <pxe/components/component.hpp>
#include <pxe/result.hpp>
#include <pxe/scenes/scene.hpp>

#include "level_manager.hpp"

#include <raylib.h>

#include <cstddef>
#include <string>

namespace energy {

class energy_swap: public pxe::app {
	static constexpr auto banner = R"(
  ______                               _____
 |  ____|                             / ____|
 | |__   _ __   ___ _ __ __ _ _   _  | (_____      ____ _ _ __
 |  __| | '_ \ / _ \ '__/ _` | | | |  \___ \ \ /\ / / _` | '_ \
 | |____| | | |  __/ | | (_| | |_| |  ____) \ V  V / (_| | |_) |
 |______|_| |_|\___|_|  \__, |\__, | |_____/ \_/\_/ \__,_| .__/
                         __/ | __/ |                     | |
                        |___/ |___/                      |_| v{})";

public:
	energy_swap(): app("energy-swap", "juan-medina", "Energy Swap", banner, design_resolution) {}
	~energy_swap() override = default;

	// Non-copyable
	energy_swap(const energy_swap &) = delete;
	auto operator=(const energy_swap &) -> energy_swap & = delete;

	// Non-movable
	energy_swap(energy_swap &&) noexcept = delete;
	auto operator=(energy_swap &&) noexcept -> energy_swap & = delete;

	// LevelManager access
	[[nodiscard]] auto get_level_manager() -> energy::level_manager & {
		return level_manager_;
	}
	[[nodiscard]] auto get_level_manager() const -> const energy::level_manager & {
		return level_manager_;
	}

	struct level_selected {
		size_t level;
	};

protected:
	[[nodiscard]] auto init() -> pxe::result<> override;
	[[nodiscard]] auto end() -> pxe::result<> override;

private:
	static constexpr auto font_path = "resources/fonts/PixeloidSans_16.fnt";
	static constexpr auto clear_color = Color{.r = 20, .g = 49, .b = 59, .a = 255};

	static constexpr auto click_sfx_path = "resources/sfx/click.wav";
	static constexpr auto click_sfx = "click";
	static constexpr auto battery_click_sfx_path = "resources/sfx/battery.wav";
	static constexpr auto battery_click_sfx = "battery";
	static constexpr auto zap_sfx_path = "resources/sfx/zap.wav";
	static constexpr auto zap_sfx = "zap";

	static constexpr pxe::size design_resolution{.width = 640, .height = 360};

	static constexpr auto levels_path = "resources/levels/levels.txt";
	static constexpr auto max_level_key = "game.max_level_reached";

	level_manager level_manager_;

	int next_level_{0};
	auto on_next_level() -> pxe::result<>;

	int game_back_{0};
	auto on_game_back() -> pxe::result<>;

	int reset_{0};
	auto on_reset_level() -> pxe::result<>;

	int level_selected_{0};
	auto on_level_selected(const level_selected &evt) -> pxe::result<>;

	int back_from_level_selection_{0};
	[[nodiscard]] auto on_back_from_level_selection() -> pxe::result<>;

	pxe::scene_id game_scene_;
	pxe::scene_id level_selection_scene_;
};

} // namespace energy