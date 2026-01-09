// SPDX-FileCopyrightText: 2026 Juan Medina
// SPDX-License-Identifier: MIT

#include "spark.hpp"

#include "../../engine/components/sprite_anim.hpp"
#include "../../engine/result.hpp"

#include <raylib.h>

#include <raymath.h>

namespace energy {

auto spark::update(const float delta) -> engine::result<> {
	if(!is_visible()) {
		return sprite_anim::update(delta);
	}
	auto current_pos = get_position();
	const auto direction = Vector2Subtract(destination_, current_pos);
	const auto distance = Vector2Length(direction);

	if(distance > 1.0F) {
		const auto move_dist = speed_ * delta;
		if(move_dist >= distance) {
			set_position(destination_);
			set_visible(false);
			stop();
		} else {
			const auto norm_dir = Vector2Normalize(direction);
			current_pos.x += norm_dir.x * move_dist;
			current_pos.y += norm_dir.y * move_dist;
			set_position(current_pos);
		}
	} else {
		set_position(destination_);
		set_visible(false);
		stop();
	}

	return sprite_anim::update(delta);
}

} // namespace energy