// SPDX-FileCopyrightText: 2026 Juan Medina
// SPDX-License-Identifier: MIT

#pragma once

#include <raylib.h>

#include <pxe/app.hpp>
#include <pxe/components/component.hpp>
#include <pxe/result.hpp>
#include <stddef.h>
#include <string>
#include <vector>

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

	[[nodiscard]] auto get_current_level() const -> size_t {
		return current_level_;
	}

	[[nodiscard]] auto get_current_level_string() const -> const std::string & {
		return levels_.at(current_level_ - 1);
	}

protected:
	[[nodiscard]] auto init() -> pxe::result<> override;
	[[nodiscard]] auto end() -> pxe::result<> override;

private:
	static constexpr auto font_path = "resources/fonts/PixeloidSans_16.fnt";
	static constexpr auto clear_color = Color{.r = 20, .g = 49, .b = 59, .a = 255};

	static constexpr auto click_sound_path = "resources/sounds/click.wav";
	static constexpr auto click_sound = "click";
	static constexpr auto battery_click_sound_path = "resources/sounds/battery.wav";
	static constexpr auto battery_click_sound = "battery";

	static constexpr pxe::size design_resolution{.width = 640, .height = 360};

	int license_scene_{-1};
	int license_accepted_{0};
	auto on_license_accepted() -> pxe::result<>;

	int menu_scene_{-1};
	int go_to_game_{0};
	auto on_go_to_game() -> pxe::result<>;

	int game_scene_{-1};

	std::vector<std::string> levels_;
	size_t current_level_{1};
	static constexpr auto levels_path = "resources/levels/levels.txt";
	auto load_levels() -> pxe::result<>;

	int next_level_{0};
	auto on_next_level() -> pxe::result<>;

	int game_back_{0};
	auto on_game_back() -> pxe::result<>;

	int reset_{0};
	auto on_reset() -> pxe::result<>;
};

} // namespace energy