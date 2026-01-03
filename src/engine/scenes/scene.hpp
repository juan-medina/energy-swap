// SPDX-FileCopyrightText: 2026 Juan Medina
// SPDX-License-Identifier: MIT

#pragma once

#include "../components/component.hpp"
#include "../result.hpp"

#include <raylib.h>

namespace engine {

class app;

class scene: public component {
public:
	scene() = default;
	~scene() override = default;

	// Non-copyable
	scene(const scene &) = delete;
	auto operator=(const scene &) -> scene & = delete;

	// Non-movable
	scene(scene &&) noexcept = delete;
	auto operator=(scene &&) noexcept -> scene & = delete;

	[[nodiscard]] auto init(app &app) -> result<> override {
		return component::init(app);
	}

	[[nodiscard]] auto end() -> result<> override {
		return component::end();
	}

	// Default update/draw do nothing and succeed.
	[[nodiscard]] auto update(float /*delta*/) -> result<> override {
		return true;
	}
	[[nodiscard]] auto draw() -> result<> override {
		return true;
	}

	// Scenes must still implement layout.
	virtual auto layout(Vector2 screen_size) -> void = 0;
};
} // namespace engine