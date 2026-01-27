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

	game(const game &) = default;
	auto operator=(const game &) -> game & = default;

	game(game &&) noexcept = default;
	auto operator=(game &&) noexcept -> game & = default;

	[[nodiscard]] auto init(pxe::app &app) -> pxe::result<> override;
	[[nodiscard]] auto end() -> pxe::result<> override;
	[[nodiscard]] auto update(float delta) -> pxe::result<> override;
	[[nodiscard]] auto layout(pxe::size screen_size) -> pxe::result<> override;
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

	// ========================================================================
	// Component State
	// ========================================================================

	size_t title_{};
	size_t status_{};
	size_t back_button_{};
	size_t next_button_{};
	size_t reset_button_{};

	// ========================================================================
	// Game State
	// ========================================================================

	puzzle current_puzzle_{};
	int battery_click_{};
	int button_click_{};

	// ========================================================================
	// Initialization
	// ========================================================================

	[[nodiscard]] auto init_ui_components() -> pxe::result<>;
	[[nodiscard]] auto init_battery_displays() -> pxe::result<>;
	[[nodiscard]] auto init_buttons() -> pxe::result<>;
	[[nodiscard]] auto init_sparks() -> pxe::result<>;

	// ========================================================================
	// Layout
	// ========================================================================

	[[nodiscard]] auto layout_title(pxe::size screen_size) const -> pxe::result<>;
	[[nodiscard]] auto layout_status(pxe::size screen_size) const -> pxe::result<>;
	[[nodiscard]] auto layout_batteries(pxe::size screen_size) const -> pxe::result<>;
	[[nodiscard]] auto layout_buttons(pxe::size screen_size) const -> pxe::result<>;

	// ========================================================================
	// Configuration
	// ========================================================================

	[[nodiscard]] auto configure_show_ui() -> pxe::result<>;
	[[nodiscard]] auto configure_button_visibility() const -> pxe::result<>;

	// ========================================================================
	// Battery Management
	// ========================================================================

	auto toggle_batteries(size_t number) -> void;
	[[nodiscard]] auto disable_all_batteries() const -> pxe::result<>;
	[[nodiscard]] auto find_selected_battery() const -> std::shared_ptr<battery_display>;
	[[nodiscard]] auto find_focussed_battery() const -> std::shared_ptr<battery_display>;

	[[nodiscard]] auto get_battery_display(size_t id) const -> pxe::result<std::shared_ptr<battery_display>>;

	// ========================================================================
	// Event Handlers
	// ========================================================================

	auto on_battery_click(const battery_display::click &click) -> pxe::result<>;
	auto on_button_click(const pxe::button::click &evt) -> pxe::result<>;

	// ========================================================================
	// Battery Click Processing
	// ========================================================================

	[[nodiscard]] auto handle_battery_selection(const std::shared_ptr<battery_display> &clicked) -> pxe::result<>;
	[[nodiscard]] auto handle_battery_transfer(const std::shared_ptr<battery_display> &selected,
											   const std::shared_ptr<battery_display> &clicked) -> pxe::result<>;
	[[nodiscard]] auto execute_energy_transfer(const std::shared_ptr<battery_display> &from,
											   const std::shared_ptr<battery_display> &to) -> pxe::result<>;

	// ========================================================================
	// Win/Lose Conditions
	// ========================================================================

	[[nodiscard]] auto check_end() -> pxe::result<>;
	[[nodiscard]] auto handle_puzzle_solved() -> pxe::result<>;
	[[nodiscard]] auto handle_puzzle_unsolvable() const -> pxe::result<>;
	[[nodiscard]] auto update_end_game_ui(const std::string &status_message, bool show_next, bool show_reset) const
		-> pxe::result<>;
	static auto constexpr win_message = "Congratulations! You completed all levels!";
	static auto constexpr continue_message = "You Win, continue to the next level ...";
	static auto constexpr unsolvable_message = "No more moves available, try again ...";

	// ========================================================================
	// Visual Effects
	// ========================================================================

	[[nodiscard]] auto shoot_sparks(Vector2 from, Vector2 to, Color color, size_t count) -> pxe::result<>;
	[[nodiscard]] auto find_free_spark() const -> std::shared_ptr<spark>;

	// ========================================================================
	// Controller Input
	// ========================================================================

	[[nodiscard]] auto update_controller_input() -> pxe::result<>;
	[[nodiscard]] auto controller_move_battery(const std::shared_ptr<battery_display> &focus) -> pxe::result<>;
	[[nodiscard]] auto move_focus_to(const std::shared_ptr<battery_display> &focus, int dx, int dy) const
		-> pxe::result<>;
	[[nodiscard]] auto should_auto_focus_battery() const -> bool;
	[[nodiscard]] auto auto_focus_first_available_battery() const -> pxe::result<>;
	[[nodiscard]] auto find_closest_battery_in_direction(const std::shared_ptr<battery_display> &focus,
														 int dx,
														 int dy) const -> std::shared_ptr<battery_display>;
	[[nodiscard]] static auto is_battery_in_direction(Vector2 focus_pos, Vector2 candidate_pos, int dx, int dy) -> bool;

	// ========================================================================
	// Solution hint
	size_t hint_from_{0};
	size_t hint_to_{0};
	bool got_hint_{false};
	bool can_have_solution_hint_{true};
	[[nodiscard]] auto set_hint_to_battery(size_t battery_num, bool is_hint) const -> pxe::result<>;
	[[nodiscard]] auto reset_hint_indicators() const -> pxe::result<>;
	[[nodiscard]] auto calculate_solution_hint() -> pxe::result<>;
};

} // namespace energy
