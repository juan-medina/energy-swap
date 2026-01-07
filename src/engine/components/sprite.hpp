// SPDX-FileCopyrightText: 2026 Juan Medina
// SPDX-License-Identifier: MIT

#pragma once

#include "../scenes/scene.hpp"
#include "component.hpp"

#include <raylib.h>

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

private:
	Color tint_ = WHITE;
	std::string sprite_sheet_;
	std::string frame_;
};
} // namespace engine