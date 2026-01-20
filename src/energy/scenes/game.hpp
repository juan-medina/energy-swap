// SPDX-FileCopyrightText: 2026 Juan Medina
// SPDX-License-Identifier: MIT

#pragma once

#include <pxe/app.hpp>
#include <pxe/components/button.hpp>
#include <pxe/components/component.hpp>
#include <pxe/result.hpp>
#include <pxe/scenes/scene.hpp>

#include "../components/battery_display.hpp"
#include "../components/spark.hpp"
#include "../data/puzzle.hpp"

#include <raylib.h>

#include <array>
#include <cstddef>
#include <memory>
#include <optional>
#include <string>

namespace pxe {
class app;
struct size;
} // namespace pxe

namespace energy {

class game: public pxe::scene {
public:
	game() = default;
	~game() override = default;

	// Copyable
	game(const game &) = default;
	auto operator=(const game &) -> game & = default;

	// Movable
	game(game &&) noexcept = default;
	auto operator=(game &&) noexcept -> game & = default;

	[[nodiscard]] auto init(pxe::app &app) -> pxe::result<> override;
	[[nodiscard]] auto end() -> pxe::result<> override;

	auto layout(pxe::size screen_size) -> pxe::result<> override;

	[[nodiscard]] auto show() -> pxe::result<> override;
	[[nodiscard]] auto reset() -> pxe::result<> override;

	[[nodiscard]] auto setup_puzzle(const std::string &puzzle_str) -> pxe::result<>;

	struct next_level {};
	struct reset_level {};
	struct back {};

private:
	static constexpr auto max_batteries = 12;
	static constexpr auto max_sparks = 25;
	static constexpr auto large_font_size = 20;
	static constexpr std::array<size_t, 12> battery_order{8, 4, 0, 1, 5, 9, 10, 6, 2, 3, 7, 11};

	static constexpr auto sprite_sheet_name = "sprites";
	static constexpr auto sprite_sheet_path = "resources/sprites/sprites.json";
	static constexpr auto game_music = "resources/music/game.ogg";
	static constexpr auto battery_click_sound = "battery";
	static constexpr auto zap_sound = "zap";

	size_t title_{};
	size_t status_{};

	std::array<size_t, max_batteries> battery_displays_{};
	puzzle current_puzzle_{};

	size_t back_button_{};
	size_t next_button_{};
	size_t reset_button_{};

	std::array<size_t, max_sparks> sparks_{};

	int battery_click_{};
	int button_click_{};

	[[nodiscard]] auto init_ui_components() -> pxe::result<>;
	[[nodiscard]] auto init_battery_displays() -> pxe::result<>;
	[[nodiscard]] auto init_buttons() -> pxe::result<>;
	[[nodiscard]] auto init_sparks() -> pxe::result<>;

	[[nodiscard]] auto layout_title(pxe::size screen_size) const -> pxe::result<>;
	[[nodiscard]] auto layout_status(pxe::size screen_size) const -> pxe::result<>;
	[[nodiscard]] auto layout_batteries(pxe::size screen_size) const -> pxe::result<>;
	[[nodiscard]] auto layout_buttons(pxe::size screen_size) const -> pxe::result<>;

	[[nodiscard]] auto configure_show_ui() -> pxe::result<>;
	[[nodiscard]] auto configure_button_visibility() const -> pxe::result<>;

	auto toggle_batteries(size_t number) -> void;
	auto disable_all_batteries() const -> void;

	auto on_battery_click(const battery_display::click &click) -> pxe::result<>;
	auto on_button_click(const pxe::button::click &evt) -> pxe::result<>;

	[[nodiscard]] auto handle_battery_selection(size_t clicked_index, battery_display &clicked_display)
		-> pxe::result<>;
	[[nodiscard]] auto handle_battery_transfer(size_t selected_index,
											   size_t clicked_index,
											   const battery_display &clicked_display) -> pxe::result<>;

	[[nodiscard]] auto find_selected_battery() const -> std::optional<size_t>;

	[[nodiscard]] auto check_end() -> pxe::result<>;
	[[nodiscard]] auto handle_puzzle_solved() -> pxe::result<>;
	[[nodiscard]] auto handle_puzzle_unsolvable() const -> pxe::result<>;

	[[nodiscard]] auto shoot_sparks(Vector2 from, Vector2 to, Color color, size_t count) -> pxe::result<>;
	[[nodiscard]] auto find_free_spark() const -> std::shared_ptr<spark>;
};

} // namespace energy