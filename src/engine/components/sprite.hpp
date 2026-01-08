// SPDX-FileCopyrightText: 2026 Juan Medina
// SPDX-License-Identifier: MIT

#pragma once

#include "../result.hpp"
#include "component.hpp"

#include <raylib.h>

#include <string>

namespace engine {

class sprite: public component {
public:
	sprite() = default;
	~sprite() override = default;

	// Non-copyable
	sprite(const sprite &) = delete;
	auto operator=(const sprite &) -> sprite & = delete;

	// Non-movable
	sprite(sprite &&) noexcept = delete;
	auto operator=(sprite &&) noexcept -> sprite & = delete;

	[[nodiscard]] auto init(app &app) -> result<> override;
	[[nodiscard]] auto init(app &app, const std::string &sprite_sheet, const std::string &frame) -> result<>;
	[[nodiscard]] auto end() -> result<> override;

	[[nodiscard]] auto update(float delta) -> result<> override;
	[[nodiscard]] auto draw() -> result<> override;

	auto set_tint(const Color &tint) -> void {
		tint_ = tint;
	}

	virtual auto set_scale(float scale) -> void;

	[[nodiscard]] auto get_scale() const -> float {
		return scale_;
	}

	[[nodiscard]] auto point_inside(Vector2 point) const -> bool override;

private:
	Color tint_ = WHITE;
	std::string sprite_sheet_;
	std::string frame_;
	float scale_ = 1.0F;

	size original_size_;
	Vector2 pivot_{};
};
} // namespace engine