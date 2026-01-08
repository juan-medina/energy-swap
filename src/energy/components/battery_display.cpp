// SPDX-FileCopyrightText: 2026 Juan Medina
// SPDX-License-Identifier: MIT

#include "battery_display.hpp"

#include "../../engine/app.hpp"

#include <spdlog/spdlog.h>

namespace energy {

auto battery_display::init(engine::app &app) -> engine::result<> {
	if(const auto err = sprite::init(app, sprite_sheet_name, battery_frame).ko(); err) {
		return engine::error("failed to initialize battery display sprite: {}", *err);
	}

	for(auto &segment: segments_) {
		if(const auto err = segment.init(app, sprite_sheet_name, full_segment_frame).ko(); err) {
			return engine::error("failed to initialize battery segment sprite: {}", *err);
		}
	}

	return true;
}

auto battery_display::draw() -> engine::result<> {
	if(!is_visible()) {
		return true;
	}
	assert(battery_.has_value() && "Battery reference not set for battery display");

	if(const auto err = sprite::draw().ko(); err) {
		return engine::error("failed to draw battery display sprite: {}", *err);
	}

	for(auto &segment: segments_) {
		if(const auto err = segment.draw().ko(); err) {
			return engine::error("failed to initialize battery segment sprite", *err);
		}
	}

	return true;
}

void battery_display::set_position(const Vector2 &pos) {
	sprite::set_position(pos);
	readjust_segments();
}

auto battery_display::update(const float delta) -> engine::result<> {
	if(!is_visible()) {
		return true;
	}
	assert(battery_.has_value() && "Battery reference not set for battery display");

	hover_ = false;
	set_tint(WHITE);

	if(const auto &bat = battery_->get(); bat.closed()) {
		set_tint(energy_colors.at(battery_->get().at(0)));
	} else {
		if(point_inside(GetMousePosition())) {
			hover_ = true;
			if(IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
				get_app().post_event(click{idx_});
			}
		}
	}

	adjust_scale();

	for(size_t i = 0; i < segments_.size(); ++i) {
		const auto color_index = battery_->get().at(i);
		const auto color = energy_colors.at(color_index);
		segments_.at(i).set_tint(color);
	}

	return sprite::update(delta);
}

void battery_display::set_scale(float scale) {
	sprite::set_scale(scale);
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
	adjust_scale();
}

auto battery_display::readjust_segments() -> void {
	const auto pos = get_position();
	Vector2 segment_pos = pos;
	segment_pos.y = pos.y + (29.0F * get_scale());
	for(auto &segment: segments_) {
		segment_pos.y -= (10.0F * get_scale());
		segment.set_position(segment_pos);
	}
}

auto battery_display::adjust_scale() -> void {
	set_scale(1.0F);
	if(selected_ && hover_) {
		set_scale(hover_selected_scale);
	} else if(selected_) {
		set_scale(selected_scale);
	} else if(hover_) {
		set_scale(hover_scale);
	}
}

} // namespace energy