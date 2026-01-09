// SPDX-FileCopyrightText: 2026 Juan Medina
// SPDX-License-Identifier: MIT

#pragma once

#include "../../engine/components/sprite_anim.hpp"
#include "../../engine/result.hpp"

#include <raylib.h>
namespace energy {

class spark: public engine::sprite_anim {
public:
	[[nodiscard]] auto init(engine::app &app) -> engine::result<> override {
		return sprite_anim::init(app, sprite_sheet, frame_pattern, total_frames, fps);
	}

	[[nodiscard]] auto update(float delta) -> engine::result<> override;

	auto set_destination(const Vector2 &dest) -> void {
		destination_ = dest;
	}

private:
	static constexpr auto sprite_sheet = "sprites";
	static constexpr auto frame_pattern = "spark_{}.png";
	static constexpr auto total_frames = 5;
	static constexpr auto fps = 15.0F;

	Vector2 destination_{};
	float speed_{200.0F};
};

} // namespace energy
