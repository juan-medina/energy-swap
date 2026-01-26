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
	// =============================================================================
	// Event Types
	// =============================================================================
	struct click {
		size_t index;
	};

	// =============================================================================
	// Lifecycle
	// =============================================================================
	[[nodiscard]] auto init(pxe::app &app) -> pxe::result<> override;
	auto reset() -> void;

	// =============================================================================
	// Update and Rendering
	// =============================================================================
	[[nodiscard]] auto update(float delta) -> pxe::result<> override;
	[[nodiscard]] auto draw() -> pxe::result<> override;

	// =============================================================================
	// Position and Scale
	// =============================================================================
	auto set_position(const Vector2 &pos) -> void override;
	auto set_scale(float scale) -> void;

	// =============================================================================
	// Battery Management
	// =============================================================================
	auto set_battery(battery &bat) -> void {
		battery_ = bat;
	}

	[[nodiscard]] auto get_top_color() const -> Color;

	// =============================================================================
	// Index Management
	// =============================================================================
	auto set_index(size_t idx) -> void {
		index_ = idx;
	}

	[[nodiscard]] auto get_index() const -> size_t {
		return index_;
	}

	// =============================================================================
	// Selection State
	// =============================================================================
	auto set_selected(bool selected) -> void;

	[[nodiscard]] auto is_selected() const -> bool {
		return selected_;
	}

	// =============================================================================
	// Hint/Next Move Flag
	// =============================================================================
	void set_next_move(const bool next_move);

private:
	// =============================================================================
	// Resource Constants
	// =============================================================================
	static constexpr auto sprite_sheet_name = "sprites";
	static constexpr auto battery_frame = "battery.png";
	static constexpr auto full_segment_frame = "full.png";
	static constexpr auto hint_frame = "hint.png";

	// =============================================================================
	// Visual Constants
	// =============================================================================
	static constexpr auto hover_scale = 1.25F;
	static constexpr auto selected_scale = 1.4F;
	static constexpr auto hover_selected_scale = 1.5F;
	static constexpr auto tint_cycle_speed = 4.0F;

	// =============================================================================
	// Input Constants
	// =============================================================================
	static constexpr auto controller_button = GAMEPAD_BUTTON_RIGHT_FACE_DOWN;

	// =============================================================================
	// Color Palette
	// =============================================================================
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

	// =============================================================================
	// Battery Data
	// =============================================================================
	std::optional<std::reference_wrapper<battery>> battery_;
	size_t index_{0};

	// =============================================================================
	// Rendering Components
	// =============================================================================
	pxe::sprite battery_sprite_;
	std::array<pxe::sprite, 4> segments_;

	// =============================================================================
	// State Management
	// =============================================================================
	bool hover_{false};
	bool selected_{false};
	float tint_progress_{0.0F};
	bool tint_increasing_{true};

	// =============================================================================
	// Controller Input
	// =============================================================================
	std::string button_frame_;
	static constexpr auto button_sheet = pxe::button::controller_sprite_list();

	// =============================================================================
	// Visual Update Methods
	// =============================================================================
	auto handle_tint(float delta) -> void;
	auto adjust_scale() -> void;
	auto readjust_segments() -> void;
	auto update_segment_colors() -> void;

	// =============================================================================
	// Input Handling
	// =============================================================================
	[[nodiscard]] auto handle_mouse_input() -> bool;
	auto handle_controller_input() -> void;

	// =============================================================================
	// State Queries
	// =============================================================================
	[[nodiscard]] auto is_battery_closed() const -> bool;
	[[nodiscard]] auto get_battery_base_color() const -> Color;
	[[nodiscard]] auto calculate_tint_color() const -> Color;

	// =============================================================================
	// Next Move Flag
	// =============================================================================
	bool next_move_{false};
	Vector2 hint_position_{0.0F, 0.0F};
};

} // namespace energy
