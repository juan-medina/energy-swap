// SPDX-FileCopyrightText: 2026 Juan Medina
// SPDX-License-Identifier: MIT

#include "battery_display.hpp"

#include <pxe/app.hpp>
#include <pxe/components/button.hpp>
#include <pxe/components/component.hpp>
#include <pxe/components/sprite.hpp>
#include <pxe/components/ui_component.hpp>
#include <pxe/result.hpp>

#include "../data/battery.hpp"

#include <raylib.h>

#include <cassert>
#include <cstddef>
#include <string>

namespace energy {

// =============================================================================
// Lifecycle Management
// =============================================================================

auto battery_display::init(pxe::app &app) -> pxe::result<> {
	if(const auto err = ui_component::init(app).unwrap(); err) {
		return pxe::error("failed to initialize base UI component", *err);
	}

	if(const auto err = battery_sprite_.init(app, sprite_sheet_name, battery_frame).unwrap(); err) {
		return pxe::error("failed to initialize battery display sprite: {}", *err);
	}

	for(auto &segment: segments_) {
		if(const auto err = segment.init(app, sprite_sheet_name, full_segment_frame).unwrap(); err) {
			return pxe::error("failed to initialize battery segment sprite: {}", *err);
		}
	}

	set_size(battery_sprite_.get_size());
	button_frame_ = pxe::button::get_controller_button_name(controller_button);

	return true;
}

auto battery_display::reset() -> void {
	hover_ = false;
	selected_ = false;
	tint_progress_ = 0.0F;
	tint_increasing_ = true;
	next_move_ = false;
	set_focussed(false);
	set_scale(1.0F);
	adjust_scale();
}

// =============================================================================
// Update and Draw
// =============================================================================

auto battery_display::update(const float delta) -> pxe::result<> {
	if(!is_visible()) {
		return true;
	}

	assert(battery_.has_value() && "Battery reference not set for battery display");

	if(const auto err = ui_component::update(delta).unwrap(); err) {
		return pxe::error("failed to update base UI component", *err);
	}

	if(const auto err = battery_sprite_.update(delta).unwrap(); err) {
		return pxe::error("failed to update battery sprite", *err);
	}

	hover_ = false;

	if(is_battery_closed()) {
		battery_sprite_.set_tint(get_battery_base_color());
	} else {
		if(handle_mouse_input()) {
			get_app().post_event(click{index_});
		}
	}

	handle_tint(delta);
	adjust_scale();
	update_segment_colors();
	handle_controller_input();

	return true;
}

auto battery_display::draw() -> pxe::result<> {
	if(!is_visible()) {
		return true;
	}

	assert(battery_.has_value() && "Battery reference not set for battery display");

	if(const auto err = battery_sprite_.draw().unwrap(); err) {
		return pxe::error("failed to draw battery display sprite: {}", *err);
	}

	for(auto &segment: segments_) {
		if(const auto err = segment.draw().unwrap(); err) {
			return pxe::error("failed to draw battery segment sprite", *err);
		}
	}

	if(is_focussed() && is_enabled()) {
		auto pos = get_position();
		const auto size = battery_sprite_.get_size();
		pos.y += (size.height / 2);
		if(const auto err = get_app().draw_sprite(button_sheet, button_frame_, pos).unwrap(); err) {
			return pxe::error("failed to draw controller button sprite", *err);
		}
	}

	if(next_move_) {
		const auto scale = battery_sprite_.get_scale();
		if(const auto err = get_app().draw_sprite(sprite_sheet_name, hint_frame, hint_position_, scale).unwrap(); err) {
			return pxe::error("failed to draw hint sprite", *err);
		}
	}

	return true;
}

// =============================================================================
// Position and Scale Management
// =============================================================================

auto battery_display::set_position(const Vector2 &pos) -> void {
	battery_sprite_.set_position(pos);
	ui_component::set_position(pos);
	readjust_segments();
}

auto battery_display::set_scale(const float scale) -> void {
	battery_sprite_.set_scale(scale);
	for(auto &segment: segments_) {
		segment.set_scale(scale);
	}
	readjust_segments();
}

auto battery_display::readjust_segments() -> void {
	const auto pos = get_position();
	const auto scale = battery_sprite_.get_scale();
	Vector2 segment_pos = pos;
	segment_pos.x += (0.5F * scale);
	segment_pos.y = pos.y + (29.0F * scale);
	for(auto &segment: segments_) {
		segment_pos.y -= (11.0F * scale);
		segment.set_position(segment_pos);
	}

	constexpr auto hint_gap = 15.0F;
	const auto [width, height] = battery_sprite_.get_size();
	hint_position_ = Vector2{.x = pos.x, .y = pos.y - (height / 2) - (hint_gap * scale)};
}

auto battery_display::adjust_scale() -> void {
	set_scale(1.0F);
	if(is_focussed() || hover_) {
		if(selected_) {
			set_scale(hover_selected_scale);
			return;
		}
		set_scale(hover_scale);
	}
}

// =============================================================================
// Selection Management
// =============================================================================

auto battery_display::set_selected(const bool selected) -> void {
	selected_ = selected;
	adjust_scale();
}

// =============================================================================
// Visual Update Methods
// =============================================================================

auto battery_display::handle_tint(const float delta) -> void {
	if(is_battery_closed()) {
		battery_sprite_.set_tint(get_battery_base_color());
		tint_progress_ = 0.0F;
		tint_increasing_ = true;
		return;
	}

	if(!selected_ || battery_->get().empty()) {
		battery_sprite_.set_tint(WHITE);
		tint_progress_ = 0.0F;
		tint_increasing_ = true;
		return;
	}

	tint_progress_ += delta * tint_cycle_speed * (tint_increasing_ ? 1.0F : -1.0F);

	if(tint_progress_ >= 1.0F) {
		tint_progress_ = 1.0F;
		tint_increasing_ = false;
	} else if(tint_progress_ <= 0.0F) {
		tint_progress_ = 0.0F;
		tint_increasing_ = true;
	}

	battery_sprite_.set_tint(calculate_tint_color());
}

auto battery_display::update_segment_colors() -> void {
	for(size_t i = 0; i < segments_.size(); ++i) {
		const auto color_index = battery_->get().at(i);
		const auto color = energy_colors.at(color_index);
		segments_.at(i).set_tint(color);
	}
}

// =============================================================================
// Input Handling
// =============================================================================

auto battery_display::handle_mouse_input() -> bool {
	if(!battery_sprite_.point_inside(GetMousePosition()) || !is_enabled()) {
		return false;
	}

	hover_ = true;
	return IsMouseButtonReleased(MOUSE_BUTTON_LEFT);
}

auto battery_display::handle_controller_input() -> void {
	if(!is_focussed()) {
		return;
	}

	if(get_app().is_controller_button_pressed(controller_button)) {
		get_app().post_event(click{index_});
	}
}

// =============================================================================
// State Queries
// =============================================================================

auto battery_display::is_battery_closed() const -> bool {
	return battery_->get().closed();
}

auto battery_display::get_battery_base_color() const -> Color {
	return energy_colors.at(battery_->get().at(0));
}

auto battery_display::get_top_color() const -> Color {
	Color result = {.r = 0, .g = 0, .b = 0, .a = 0};
	if(battery_.has_value()) {
		const auto color_index = battery_->get().at(battery_->get().size() - 1);
		result = energy_colors.at(color_index);
	}
	return result;
}

auto battery_display::calculate_tint_color() const -> Color {
	const auto top_color = get_top_color();
	return ColorLerp(WHITE, top_color, 0.25F + (tint_progress_ * 0.75F));
}

void battery_display::set_next_move(const bool next_move) {
	next_move_ = next_move;
}

} // namespace energy
