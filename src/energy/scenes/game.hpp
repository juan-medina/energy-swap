// SPDX-FileCopyrightText: 2026 Juan Medina
// SPDX-License-Identifier: MIT

#pragma once

#include "../../engine/app.hpp"
#include "../../engine/components/button.hpp"
#include "../../engine/components/component.hpp"
#include "../../engine/components/label.hpp"
#include "../../engine/result.hpp"
#include "../../engine/scenes/scene.hpp"
#include "../components/battery_display.hpp"
#include "../components/spark.hpp"
#include "../data/battery.hpp"
#include "../data/puzzle.hpp"

#include <raylib.h>
#include <raymath.h>

#include <array>
#include <cstddef>
#include <memory>
#include <string>

namespace engine {
class app;
struct size;
} // namespace engine

namespace energy {

class game: public engine::scene {
public:
	game() = default;
	~game() override = default;

	// Copyable
	game(const game &) = default;
	auto operator=(const game &) -> game & = default;

	// Movable
	game(game &&) noexcept = default;
	auto operator=(game &&) noexcept -> game & = default;

	[[nodiscard]] auto init(engine::app &app) -> engine::result<> override;
	[[nodiscard]] auto end() -> engine::result<> override;

	[[nodiscard]] auto update(float delta) -> engine::result<> override;
	[[nodiscard]] auto draw() -> engine::result<> override;

	auto layout(engine::size screen_size) -> engine::result<> override;

	[[nodiscard]] auto setup_puzzle(const std::string &puzzle_str) -> engine::result<>;

	[[nodiscard]] auto enable() -> engine::result<> override;

	struct next_level {};
	struct back {};
	struct reset {};

private:
	engine::label title_;
	engine::label status_;
	static constexpr auto large_font_size = 20;

	static constexpr auto max_batteries = 12;
	std::array<battery_display, max_batteries> battery_displays_;
	std::array<battery, max_batteries> batteries_;

	static constexpr auto sprite_sheet_name = "sprites";
	static constexpr auto sprite_sheet_path = "resources/sprites/sprites.json";
	static constexpr auto game_music = "resources/music/game.ogg";
	static constexpr auto battery_click_sound = "battery";

	auto toggle_batteries(size_t number) -> void;
	static constexpr std::array<size_t, 12> battery_order{8, 4, 0, 1, 5, 9, 10, 6, 2, 3, 7, 11};

	puzzle current_puzzle_;

	int battery_click_{0};
	auto on_battery_click(const battery_display::click &click) -> engine::result<>;

	engine::button back_button_;
	engine::button next_button_;
	engine::button reset_button_;

	int button_click_{0};
	auto on_button_click(const engine::button::click &evt) -> engine::result<>;

	auto check_end() -> void;

	static auto constexpr max_sparks = 25;
	std::array<size_t, max_sparks> sparks_;

	auto find_free_spark() -> std::shared_ptr<spark>;

	auto shoot_sparks(Vector2 from, Vector2 to, Color color, size_t count) -> void;
};

} // namespace energy