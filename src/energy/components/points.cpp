// SPDX-FileCopyrightText: 2026 Juan Medina
// SPDX-License-Identifier: MIT

#include "points.hpp"

#include <pxe/components/label.hpp>
#include <pxe/result.hpp>

#include <raylib.h>

#include <string>

namespace energy {

auto points::set_points(const int points) -> void {
	set_text("+" + std::to_string(points) + "s");
	set_visible(true);
	time_to_live_ = time_to_live;
	set_text_color(full_green);
}

auto points::update(float delta_time) -> pxe::result<> {
	if(const auto err = label::update(delta_time).unwrap(); err) {
		return pxe::error("failed to update base label component", *err);
	}

	if(!is_visible() || !is_enabled()) {
		return true;
	}

	time_to_live_ -= delta_time;
	if(time_to_live_ <= 0.0F) {
		set_visible(false);
	} else {
		auto position = get_position();
		position.y -= up_speed * delta_time;
		set_position(position);
		set_text_color(ColorLerp(fade_green, full_green, time_to_live_ / time_to_live));
	}

	return true;
}

} // namespace energy