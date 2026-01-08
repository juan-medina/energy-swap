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

	[[nodiscard]] virtual auto enable() -> result<> {
		return true;
	}

	[[nodiscard]] virtual auto disable() -> result<> {
		return true;
	}

	[[nodiscard]] auto update(float /*delta*/) -> result<> override {
		return true;
	}
	[[nodiscard]] auto draw() -> result<> override {
		return true;
	}

	virtual auto layout(size screen_size) -> void = 0;
};
} // namespace engine