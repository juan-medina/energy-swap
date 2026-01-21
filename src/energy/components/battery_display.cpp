// SPDX-FileCopyrightText: 2026 Juan Medina
// SPDX-License-Identifier: MIT

#include "battery_display.hpp"

#include <pxe/app.hpp>
#include <pxe/components/button.hpp>
#include <pxe/components/sprite.hpp>
#include <pxe/components/ui_component.hpp>
#include <pxe/result.hpp>

#include "../data/battery.hpp"

#include <raylib.h>

#include <cassert>
#include <cstddef>

namespace energy {

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
			return pxe::error("failed to initialize battery segment sprite", *err);
		}
	}

	if(is_focussed() && is_enabled()) {
		auto pos = get_position();
		auto size = battery_sprite_.get_size();
		pos.y += (size.height / 2);
		if(const auto err = get_app().draw_sprite(button_sheet, button_frame_, pos).unwrap(); err) {
			return pxe::error("failed to draw controller button sprite", *err);
		}
	}

	return true;
}

void battery_display::set_position(const Vector2 &pos) {
	battery_sprite_.set_position(pos);
	ui_component::set_position(pos);
	readjust_segments();
}

auto battery_display::update(const float delta) -> pxe::result<> {
	if(!is_visible()) {
		return true;
	}
	assert(battery_.has_value() && "Battery reference not set for battery display");

	if(const auto err = ui_component::update(delta).unwrap(); err) {
		return pxe::error("failed to update base UI component", *err);
	}

	if(const auto err = battery_sprite_.update(delta).unwrap(); err) {
		return pxe::error("failed to update base UI component", *err);
	}

	hover_ = false;

	handle_tint(delta);

	if(const auto &bat = battery_->get(); bat.closed()) {
		battery_sprite_.set_tint(energy_colors.at(battery_->get().at(0)));
	} else {
		if(battery_sprite_.point_inside(GetMousePosition()) && is_enabled()) {
			hover_ = true;
			if(IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
				get_app().post_event(click{index_});
			}
		}
	}

	adjust_scale();

	for(size_t i = 0; i < segments_.size(); ++i) {
		const auto color_index = battery_->get().at(i);
		const auto color = energy_colors.at(color_index);
		segments_.at(i).set_tint(color);
	}

	if(is_focussed()) {
		if(get_app().is_controller_button_pressed(controller_button)) {
			get_app().post_event(click{index_});
		}
	}

	return true;
}

void battery_display::set_scale(const float scale) {
	battery_sprite_.set_scale(scale);
	for(auto &segment: segments_) {
		segment.set_scale(scale);
	}
	readjust_segments();
}

auto battery_display::set_selected(const bool selected) -> void {
	selected_ = selected;
	adjust_scale();
}

auto battery_display::reset() -> void {
	hover_ = false;
	selected_ = false;
	tint_progress_ = 0.0F;
	tint_increasing_ = true;
	set_focussed(false);
	set_scale(1.0F);
	adjust_scale();
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

auto battery_display::handle_tint(const float delta) -> void {
	if(const auto &bat = battery_->get(); bat.closed()) {
		battery_sprite_.set_tint(energy_colors.at(battery_->get().at(0)));
		tint_progress_ = 0.0F;
		tint_increasing_ = true;
	} else if(selected_ && !bat.empty()) {
		// Animate tint between WHITE and top color
		tint_progress_ += delta * tint_cycle_speed * (tint_increasing_ ? 1.0F : -1.0F);

		if(tint_progress_ >= 1.0F) {
			tint_progress_ = 1.0F;
			tint_increasing_ = false;
		} else if(tint_progress_ <= 0.0F) {
			tint_progress_ = 0.0F;
			tint_increasing_ = true;
		}

		const auto top_color = get_top_color();
		battery_sprite_.set_tint(ColorLerp(WHITE, top_color, 0.25F + (tint_progress_ * 0.75F)));
	} else {
		battery_sprite_.set_tint(WHITE);
		tint_progress_ = 0.0F;
		tint_increasing_ = true;
	}
}

} // namespace energy