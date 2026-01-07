// SPDX-FileCopyrightText: 2026 Juan Medina
// SPDX-License-Identifier: MIT

#include "battery_display.hpp"

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
	Vector2 segment_pos = pos;
	segment_pos.y = pos.y + 30.0F;
	for(auto &segment: segments_) {
		segment_pos.y -= 10.0F;
		segment.set_position(segment_pos);
	}
}
auto battery_display::update(const float delta) -> engine::result<> {
	if(!is_visible()) {
		return true;
	}
	assert(battery_.has_value() && "Battery reference not set for battery display");

	set_tint(WHITE);
	if(const auto &bat = battery_->get(); bat.closed()) {
		set_tint(energy_colors.at(battery_->get().at(0)));
	}

	for(size_t i = 0; i < segments_.size(); ++i) {
		const auto color_index = battery_->get().at(static_cast<int>(i));
		const auto color = energy_colors.at(color_index);
		segments_.at(i).set_tint(color);
	}

	return sprite::update(delta);
}

} // namespace energy