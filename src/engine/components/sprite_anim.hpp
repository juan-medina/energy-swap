// SPDX-FileCopyrightText: 2026 Juan Medina
// SPDX-License-Identifier: MIT

#pragma once

#include "../result.hpp"
#include "component.hpp"
#include "sprite.hpp"

#include <cassert>
#include <string>

namespace engine {
class app;

class sprite_anim: public sprite {
public:
	sprite_anim();
	~sprite_anim() override;

	// Copyable
	sprite_anim(const sprite_anim &) = default;
	auto operator=(const sprite_anim &) -> sprite_anim & = default;

	// Movable
	sprite_anim(sprite_anim &&) noexcept = default;
	auto operator=(sprite_anim &&) noexcept -> sprite_anim & = default;

	[[nodiscard]] auto init(app &app) -> result<> override;
	[[nodiscard]] auto init(app &app, const std::string &sprite_sheet, const std::string &frame) -> result<> override;
	[[nodiscard]] auto
	init(app &app, const std::string &sprite_sheet, const std::string &pattern, int frames, float fps) -> result<>;
	[[nodiscard]] auto update(float delta) -> result<> override;

	auto reset() -> result<>;

	auto play() -> void;
	auto stop() -> void;

private:
	bool running_{false};
	std::string frame_pattern_;
	int frames_ = 1;
	int current_frame_ = 1;
	float fps_ = 1.0F;
	float time_accum_ = 0.0F;

	void update_frame_name();
};

} // namespace engine
