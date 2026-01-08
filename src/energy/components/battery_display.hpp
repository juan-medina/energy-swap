// SPDX-FileCopyrightText: 2026 Juan Medina
// SPDX-License-Identifier: MIT

#pragma once

#include "../../engine/components/sprite.hpp"
#include "../data/battery.hpp"

#include <array>
#include <optional>

namespace energy {

class battery_display: public engine::sprite {
public:
	auto init(engine::app &app) -> engine::result<> override;

	static constexpr auto sprite_sheet_name = "sprites";
	static constexpr auto battery_frame = "battery.png";
	static constexpr auto full_segment_frame = "full.png";

	auto set_battery(battery &bat) -> void {
		battery_ = bat;
	}

	[[nodiscard]] auto draw() -> engine::result<> override;
	auto set_position(const Vector2 &pos) -> void override;
	[[nodiscard]] auto update(float delta) -> engine::result<> override;

	auto set_scale(float scale) -> void override;

	auto set_id(const size_t &idx) -> void {
		idx_ = idx;
	}

	[[nodiscard]] auto get_id() const -> size_t {
		return idx_;
	}

	[[nodiscard]] auto is_selected() const -> bool {
		return selected_;
	}

	auto set_selected(bool selected) -> void;

	struct click {
		size_t index;
	};

	auto reset() -> void;

private:
	static constexpr auto hover_scale = 1.25F;
	static constexpr auto selected_scale = 1.4F;
	static constexpr auto hover_selected_scale = 1.5F;

	std::optional<std::reference_wrapper<battery>> battery_;
	std::array<sprite, 4> segments_;

	static constexpr std::array<Color, 11> energy_colors = {{
		{.r = 0xD0, .g = 0x00, .b = 0x00, .a = 0x00}, // transparent
		{.r = 0xD6, .g = 0x27, .b = 0x28, .a = 0xFF}, // red
		{.r = 0xFF, .g = 0xD7, .b = 0x00, .a = 0xFF}, // gold
		{.r = 0x17, .g = 0xBE, .b = 0xCF, .a = 0xFF}, // cyan
		{.r = 0x94, .g = 0x67, .b = 0xBD, .a = 0xFF}, // purple
		{.r = 0x2C, .g = 0xA0, .b = 0x2C, .a = 0xFF}, // green
		{.r = 0xFF, .g = 0x7F, .b = 0x0E, .a = 0xFF}, // orange
		{.r = 0x8C, .g = 0x56, .b = 0x4B, .a = 0xFF}, // brown
		{.r = 0x00, .g = 0x80, .b = 0x80, .a = 0xFF}, // teal
		{.r = 0xE3, .g = 0x77, .b = 0xC2, .a = 0xFF}, // pink
		{.r = 0x1F, .g = 0x77, .b = 0xB4, .a = 0xFF}, // blue
	}};

	auto readjust_segments() -> void;
	auto adjust_scale() -> void;

	bool hover_ = false;
	bool selected_ = false;
	size_t idx_ = 0;
};

} // namespace energy
