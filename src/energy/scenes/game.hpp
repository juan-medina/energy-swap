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
#include "../data/battery.hpp"
#include "../data/puzzle.hpp"

#include <raylib.h>

#include <array>
#include <cstddef>
#include <memory>
#include <raymath.h>
#include <string>

namespace pxe {
class app;
struct size;
} // namespace pxe

namespace engine {
class app;
struct size;
} // namespace engine

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

	[[nodiscard]] auto setup_puzzle(const std::string &puzzle_str) -> pxe::result<>;

	[[nodiscard]] auto show() -> pxe::result<> override;

	[[nodiscard]] auto reset() -> pxe::result<> override;

	struct next_level {};
	struct reset_level {};
	struct back {};

private:
	size_t title_;
	size_t status_;
	static constexpr auto large_font_size = 20;

	static constexpr auto max_batteries = 12;
	std::array<size_t, max_batteries> battery_displays_;
	std::array<battery, max_batteries> batteries_;

	static constexpr auto sprite_sheet_name = "sprites";
	static constexpr auto sprite_sheet_path = "resources/sprites/sprites.json";
	static constexpr auto game_music = "resources/music/game.ogg";
	static constexpr auto battery_click_sound = "battery";
	static constexpr auto zap_sound = "zap";

	auto toggle_batteries(size_t number) -> void;
	static constexpr std::array<size_t, 12> battery_order{8, 4, 0, 1, 5, 9, 10, 6, 2, 3, 7, 11};

	puzzle current_puzzle_;

	int battery_click_{0};
	auto on_battery_click(const battery_display::click &click) -> pxe::result<>;

	size_t back_button_{0};
	size_t next_button_{0};
	size_t reset_button_{0};

	int button_click_{0};
	auto on_button_click(const pxe::button::click &evt) -> pxe::result<>;

	[[nodiscard]] auto check_end() -> pxe::result<>;

	static auto constexpr max_sparks = 25;
	std::array<size_t, max_sparks> sparks_{};

	auto find_free_spark() -> std::shared_ptr<spark>;

	[[nodiscard]] auto shoot_sparks(Vector2 from, Vector2 to, Color color, size_t count) -> pxe::result<>;
};

} // namespace energy