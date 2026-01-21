// SPDX-FileCopyrightText: 2026 Juan Medina
// SPDX-License-Identifier: MIT

#pragma once

#include <pxe/app.hpp>
#include <pxe/components/button.hpp>
#include <pxe/components/component.hpp>
#include <pxe/result.hpp>
#include <pxe/scenes/scene.hpp>

#include <array>
#include <cstddef>

namespace pxe {
class app;
struct size;
} // namespace pxe

namespace energy {

class level_selection: public pxe::scene {
public:
	level_selection() = default;
	~level_selection() override = default;

	// Copyable
	level_selection(const level_selection &) = default;
	auto operator=(const level_selection &) -> level_selection & = default;

	// Movable
	level_selection(level_selection &&) noexcept = default;
	auto operator=(level_selection &&) noexcept -> level_selection & = default;

	[[nodiscard]] auto init(pxe::app &app) -> pxe::result<> override;
	[[nodiscard]] auto end() -> pxe::result<> override;
	[[nodiscard]] auto update(float delta) -> pxe::result<> override;

	auto layout(pxe::size screen_size) -> pxe::result<> override;

	[[nodiscard]] auto show() -> pxe::result<> override;

private:
	size_t title_{0};

	static constexpr auto max_levels = 100;
	static constexpr auto levels_per_page = 10;
	static constexpr auto total_pages = max_levels / levels_per_page;
	static constexpr auto button_font_size = 16;

	size_t current_page_{0};

	static constexpr auto max_level_buttons = levels_per_page;
	std::array<size_t, max_level_buttons> level_buttons_{};

	size_t prev_page_button_{0};
	size_t next_page_button_{0};
	size_t back_button_{0};

	int button_click_{0};
	auto on_button_click(const pxe::button::click &evt) -> pxe::result<>;

	auto update_buttons() const -> pxe::result<>;

	static constexpr auto max_reached_level_key = "max_reached_level";
	size_t max_reached_level_{1};

	size_t selected_level_{1};
	auto on_dpad_input(int dx, int dy) -> pxe::result<>;
	static constexpr auto click_sfx = "click";
	auto controller_move_level() -> pxe::result<>;
	auto check_page_movement() -> pxe::result<>;
};

} // namespace energy