// SPDX-FileCopyrightText: 2026 Juan Medina
// SPDX-License-Identifier: MIT

#pragma once

#include "../engine/app.hpp"

namespace energy {

class energy_swap: public engine::app {
public:
	energy_swap(): app("Energy Swapper") {}
	~energy_swap() override = default;

	// Non-copyable
	energy_swap(const energy_swap &) = delete;
	auto operator=(const energy_swap &) -> energy_swap & = delete;

	// Non-movable
	energy_swap(energy_swap &&) noexcept = delete;
	auto operator=(energy_swap &&) noexcept -> energy_swap & = delete;

protected:
	[[nodiscard]] auto init() -> engine::result<> override;
	[[nodiscard]] auto end() -> engine::result<> override;

private:
	static constexpr auto font_path = "resources/fonts/PixeloidSans-mLxMm.ttf";
	static constexpr auto load_font_size = 40;
	static constexpr auto default_font_size = 20;
	static constexpr Color clear_color = Color{.r = 20, .g = 49, .b = 59, .a = 255};

	auto on_license_accepted() -> void;
	int license_scene_{-1};
	engine::event_bus::token_t license_accepted_{0};

	int menu_scene_{-1};

	static constexpr auto click_sound_path = "resources/sounds/click.wav";
	static constexpr auto click_sound = "click";
};

} // namespace energy