// SPDX-FileCopyrightText: 2026 Juan Medina
// SPDX-License-Identifier: MIT

#pragma once

#include "../../engine/app.hpp"
#include "../../engine/components/button.hpp"
#include "../../engine/events.hpp"
#include "../../engine/result.hpp"
#include "../../engine/scenes/scene.hpp"

namespace energy {

class menu: public engine::scene {
public:
	menu() = default;
	~menu() override = default;

	// Non-copyable
	menu(const menu &) = delete;
	auto operator=(const menu &) -> menu & = delete;

	// Non-movable
	menu(menu &&) noexcept = delete;
	auto operator=(menu &&) noexcept -> menu & = delete;

	[[nodiscard]] auto init(engine::app &app) -> engine::result<> override;
	[[nodiscard]] auto end() -> engine::result<> override;

	[[nodiscard]] auto update(float delta) -> engine::result<> override;
	[[nodiscard]] auto draw() -> engine::result<> override;

	auto layout(Vector2 screen_size) -> void override;

private:
	engine::button play_button_;
	engine::event_bus::token_t button_click_{0};

	auto on_button_click(const engine::button::click &evt) const -> void;
};

} // namespace energy