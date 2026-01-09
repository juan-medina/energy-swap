// SPDX-FileCopyrightText: 2026 Juan Medina
// SPDX-License-Identifier: MIT

#pragma once

#include "../../engine/app.hpp"
#include "../../engine/components/button.hpp"
#include "../../engine/components/component.hpp"
#include "../../engine/components/label.hpp"
#include "../../engine/result.hpp"
#include "../../engine/scenes/scene.hpp"

namespace engine {
class app;
struct size;
} // namespace engine

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

	auto layout(engine::size screen_size) -> engine::result<> override;

	struct go_to_game {};

	auto enable() -> engine::result<> override;

private:
	engine::label title_;
	static constexpr auto large_font_size = 20;
	static constexpr auto menu_music_path = "resources/music/menu.ogg";

	engine::button play_button_;
	int button_click_{0};

	auto on_button_click(const engine::button::click &evt) -> engine::result<>;
};

} // namespace energy