// SPDX-FileCopyrightText: 2026 Juan Medina
// SPDX-License-Identifier: MIT

#pragma once

#include <pxe/app.hpp>
#include <pxe/components/button.hpp>
#include <pxe/components/sprite.hpp>
#include <pxe/components/ui_component.hpp>
#include <pxe/result.hpp>

#include "../data/battery.hpp"

#include <raylib.h>

#include <array>
#include <cstddef>
#include <functional>
#include <optional>
#include <string>

namespace pxe {
class app;
} // namespace pxe

namespace energy {
class battery;

class battery_display: public pxe::ui_component {
public:
	auto init(pxe::app &app) -> pxe::result<> override;

	static constexpr auto sprite_sheet_name = "sprites";
	static constexpr auto battery_frame = "battery.png";
	static constexpr auto full_segment_frame = "full.png";
	static constexpr auto controller_sheet_name = "menu";
	static constexpr auto controller_button_frame = "button_07.png";

	auto set_battery(battery &bat) -> void {
		battery_ = bat;
	}

	[[nodiscard]] auto draw() -> pxe::result<> override;
	auto set_position(const Vector2 &pos) -> void override;
	[[nodiscard]] auto update(float delta) -> pxe::result<> override;

	auto set_scale(float scale) -> void;

	auto set_index(const size_t &idx) -> void {
		index_ = idx;
	}

	[[nodiscard]] auto get_index() const -> size_t {
		return index_;
	}

	[[nodiscard]] auto is_selected() const -> bool {
		return selected_;
	}

	auto set_selected(bool selected) -> void;

	struct click {
		size_t index;
	};

	auto reset() -> void;

	[[nodiscard]] auto get_top_color() const -> Color {
		Color result = {.r = 0, .g = 0, .b = 0, .a = 0};
		if(battery_.has_value()) {
			const auto color_index = battery_->get().at(battery_->get().size() - 1);
			result = energy_colors.at(color_index);
		}

		return result;
	}

private:
	static constexpr auto hover_scale = 1.25F;
	static constexpr auto selected_scale = 1.4F;
	static constexpr auto hover_selected_scale = 1.5F;
	static constexpr auto tint_cycle_speed = 4.0F;

	float tint_progress_ = 0.0F;
	bool tint_increasing_ = true;

	std::optional<std::reference_wrapper<battery>> battery_;
	std::array<pxe::sprite, 4> segments_;
	pxe::sprite battery_sprite_;

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
	size_t index_ = 0;
	static auto constexpr controller_button = GAMEPAD_BUTTON_RIGHT_FACE_DOWN;
	static auto constexpr button_sheet = pxe::button::controller_sprite_list();
	std::string button_frame_;

	auto handle_tint(float delta) -> void;
};

} // namespace energy
