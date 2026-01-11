// SPDX-FileCopyrightText: 2026 Juan Medina
// SPDX-License-Identifier: MIT

#pragma once

#include "../../engine/app.hpp"
#include "../../engine/components/component.hpp"
#include "../../engine/result.hpp"
#include "../../engine/scenes/scene.hpp"
#include "../../engine/components/button.hpp"

#include <raylib.h>

#include <cstddef>

namespace energy {

class hud: public engine::scene {
public:
	[[nodiscard]] auto init(engine::app &app) -> engine::result<> override;
	[[nodiscard]] auto layout(engine::size size) -> engine::result<> override;
	[[nodiscard]] auto end() -> engine::result<> override;

private:
	size_t quick_bar_{};

	static constexpr auto sprite_sheet = "sprites";
	static constexpr auto normal = Color{.r = 0xFF, .g = 0xFF, .b = 0xFF, .a = 0x3C};
	static constexpr auto hover = Color{.r = 0xFF, .g = 0xFF, .b = 0xFF, .a = 0x7F};
	static constexpr auto gap = 5.0F;

	size_t close_button_{};
	size_t toggle_fullscreen_button_{};

	int button_click_{0};
	auto on_button_click(const engine::button::click &evt) -> engine::result<>;

	static constexpr auto fullscreen_frame = "larger.png";
	static constexpr auto windowed_frame = "smaller.png";
};

} // namespace energy
