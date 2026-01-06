// SPDX-FileCopyrightText: 2026 Juan Medina
// SPDX-License-Identifier: MIT

#pragma once

#include "../../engine/app.hpp"
#include "../../engine/components/button.hpp"
#include "../../engine/events.hpp"
#include "../../engine/result.hpp"
#include "../../engine/scenes/scene.hpp"
#include "../../engine/components/label.hpp"

namespace energy {

class game: public engine::scene {
public:
	game() = default;
	~game() override = default;

	// Non-copyable
	game(const game &) = delete;
	auto operator=(const game &) -> game & = delete;

	// Non-movable
	game(game &&) noexcept = delete;
	auto operator=(game &&) noexcept -> game & = delete;

	[[nodiscard]] auto init(engine::app &app) -> engine::result<> override;
	[[nodiscard]] auto end() -> engine::result<> override;

	[[nodiscard]] auto update(float delta) -> engine::result<> override;
	[[nodiscard]] auto draw() -> engine::result<> override;

	auto layout(engine::size screen_size) -> void override;

private:
	engine::label title_;
	static constexpr auto large_font_size = 20;
};

} // namespace energy