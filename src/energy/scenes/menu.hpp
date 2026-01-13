// SPDX-FileCopyrightText: 2026 Juan Medina
// SPDX-License-Identifier: MIT

#pragma once

#include <pxe/app.hpp>
#include <pxe/components/button.hpp>
#include <pxe/components/component.hpp>
#include <pxe/components/label.hpp>
#include <pxe/result.hpp>
#include <pxe/scenes/scene.hpp>

namespace engine {
class app;
struct size;
} // namespace engine

namespace energy {

class menu: public pxe::scene {
public:
	menu() = default;
	~menu() override = default;

	// Copyable
	menu(const menu &) = default;
	auto operator=(const menu &) -> menu & = default;

	// Movable
	menu(menu &&) noexcept = default;
	auto operator=(menu &&) noexcept -> menu & = default;

	[[nodiscard]] auto init(pxe::app &app) -> pxe::result<> override;
	[[nodiscard]] auto end() -> pxe::result<> override;

	[[nodiscard]] auto update(float delta) -> pxe::result<> override;
	[[nodiscard]] auto draw() -> pxe::result<> override;

	auto layout(pxe::size screen_size) -> pxe::result<> override;

	struct go_to_game {};

	auto enable() -> pxe::result<> override;

private:
	pxe::label title_;
	static constexpr auto large_font_size = 20;
	static constexpr auto menu_music_path = "resources/music/menu.ogg";

	pxe::button play_button_;
	int button_click_{0};

	auto on_button_click(const pxe::button::click &evt) -> pxe::result<>;
};

} // namespace energy