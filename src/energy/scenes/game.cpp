// SPDX-FileCopyrightText: 2026 Juan Medina
// SPDX-License-Identifier: MIT

#include "game.hpp"

#include <pxe/app.hpp>
#include <pxe/components/button.hpp>
#include <pxe/components/component.hpp>
#include <pxe/components/label.hpp>
#include <pxe/result.hpp>
#include <pxe/scenes/scene.hpp>

#include "../components/battery_display.hpp"
#include "../components/spark.hpp"
#include "../data/battery.hpp"
#include "../data/puzzle.hpp"
#include "../energy_swap.hpp"
#include "spdlog/fmt/bundled/base.h"

#include <raylib.h>

#include <array>
#include <cstddef>
#include <format>
#include <functional>
#include <memory>
#include <optional>
#include <ranges>
#include <spdlog/spdlog.h>

namespace energy {

auto game::init(pxe::app &app) -> pxe::result<> {
	if(const auto err = scene::init(app).unwrap(); err) {
		return pxe::error("failed to initialize base component", *err);
	}

	SPDLOG_INFO("game scene initialized");

	if(const auto err = title_.init(app).unwrap(); err) {
		return pxe::error("failed to initialize title label", *err);
	}

	if(const auto err = status_.init(app).unwrap(); err) {
		return pxe::error("failed to initialize status label", *err);
	}

	if(const auto err = back_button_.init(app).unwrap(); err) {
		return pxe::error("failed to initialize back button", *err);
	}

	if(const auto err = next_button_.init(app).unwrap(); err) {
		return pxe::error("failed to initialize next button", *err);
	}

	if(const auto err = reset_button_.init(app).unwrap(); err) {
		return pxe::error("failed to initialize reset button", *err);
	}

	if(const auto err = app.load_sprite_sheet(sprite_sheet_name, sprite_sheet_path).unwrap(); err) {
		return pxe::error("failed to initialize sprite sheet", *err);
	}

	for(auto &battery_display: battery_displays_) {
		if(const auto err = battery_display.init(app).unwrap(); err) {
			return pxe::error("failed to initialize battery sprite", *err);
		}
		battery_display.set_visible(false);
	}

	for(const auto index: std::views::iota(0, max_batteries)) {
		const auto ordered = battery_order.at(index);
		battery_displays_.at(index).set_battery(batteries_.at(ordered));
		battery_displays_.at(index).set_index(index);
	}

	battery_click_ = app.bind_event<battery_display::click>(this, &game::on_battery_click);

	back_button_.set_text("Back");
	back_button_.set_position({.x = 0, .y = 0});
	back_button_.set_size({.width = 45, .height = 25});

	next_button_.set_text("Next");
	next_button_.set_position({.x = 0, .y = 0});
	next_button_.set_size({.width = 45, .height = 25});

	reset_button_.set_text("Reset");
	reset_button_.set_position({.x = 0, .y = 0});
	reset_button_.set_size({.width = 45, .height = 25});

	button_click_ = app.bind_event<pxe::button::click>(this, &game::on_button_click);

	// init all sparks
	for(auto &id: sparks_) {
		if(auto err = register_component<spark>().unwrap(id); err) {
			return pxe::error("failed to register spark animation", *err);
		}

		std::shared_ptr<spark> spark_ptr;
		if(auto err = get_component<spark>(id).unwrap(spark_ptr); err) {
			return pxe::error("failed to get spark animation", *err);
		}
		spark_ptr->set_scale(2.0F);
		spark_ptr->set_visible(false);
	}
	return true;
}

auto game::end() -> pxe::result<> {
	if(const auto err = title_.end().unwrap(); err) {
		return pxe::error("failed to end title label", *err);
	}

	if(const auto err = status_.end().unwrap(); err) {
		return pxe::error("failed to end status label", *err);
	}

	if(const auto err = back_button_.end().unwrap(); err) {
		return pxe::error("failed to end back button", *err);
	}

	if(const auto err = next_button_.end().unwrap(); err) {
		return pxe::error("failed to end next button", *err);
	}

	if(const auto err = reset_button_.end().unwrap(); err) {
		return pxe::error("failed to end reset button", *err);
	}

	if(const auto err = get_app().unload_sprite_sheet(sprite_sheet_name).unwrap(); err) {
		return pxe::error("failed to end sprite sheet", *err);
	}

	for(auto &sprite: battery_displays_) {
		if(const auto err = sprite.end().unwrap(); err) {
			return pxe::error("failed to end battery display", *err);
		}
	}

	get_app().unsubscribe(button_click_);

	return scene::end();
}

auto game::update(const float delta) -> pxe::result<> {
	for(auto &sprite: battery_displays_) {
		if(const auto err = sprite.update(delta).unwrap(); err) {
			return pxe::error("failed to update battery display", *err);
		}
	}

	return scene::update(delta);
}

auto game::draw() -> pxe::result<> {
	if(const auto err = title_.draw().unwrap(); err) {
		return pxe::error("failed to draw title label", *err);
	}

	if(const auto err = status_.draw().unwrap(); err) {
		return pxe::error("failed to draw status label", *err);
	}

	if(const auto err = back_button_.draw().unwrap(); err) {
		return pxe::error("failed to draw back button", *err);
	}

	if(const auto err = next_button_.draw().unwrap(); err) {
		return pxe::error("failed to draw next button", *err);
	}

	if(const auto err = reset_button_.draw().unwrap(); err) {
		return pxe::error("failed to draw reset button", *err);
	}

	for(auto &sprite: battery_displays_) {
		if(const auto err = sprite.draw().unwrap(); err) {
			return pxe::error("failed to draw battery sprite", *err);
		}
	}

	return scene::draw();
}

auto game::layout(const pxe::size screen_size) -> pxe::result<> {
	title_.set_position({
		.x = screen_size.width / 2.0F,
		.y = 10.0F,
	});

	status_.set_position({
		.x = screen_size.width / 2.0F,
		.y = screen_size.height - 60.0F,
	});

	// distribute batteries in a grid of 2 rows that fills
	// 80% of the screen width and 70% of the screen height
	constexpr int rows = 2;
	constexpr int cols = max_batteries / rows;
	const auto horizontal_space = screen_size.width * 0.8F;
	const auto vertical_space = screen_size.height * 0.7F;
	const auto battery_width = horizontal_space / cols;
	const auto battery_height = vertical_space / rows;
	const auto start_x = (screen_size.width - horizontal_space) / 2.0F;
	const auto start_y = (screen_size.height - vertical_space) / 2.0F;

	for(int i = 0; i < max_batteries; ++i) {
		auto const row = i / cols;
		auto const col = i % cols;
		auto const pos_x = start_x + (battery_width * static_cast<float>(col)) + (battery_width / 2.0F);
		auto const pos_y = start_y + (battery_height * static_cast<float>(row)) + (battery_height / 2.0F);
		battery_displays_.at(i).set_position({.x = pos_x, .y = pos_y});
	}

	constexpr auto button_v_gap = 10.0F;
	constexpr auto button_h_gap = 10.0F;

	const auto [button_width, button_height] = back_button_.get_size(); // assuming all buttons have the same size

	const auto center_pos_x = screen_size.width * 0.5F;
	const auto center_pos_y = screen_size.height - button_height - button_v_gap;

	back_button_.set_position({
		.x = center_pos_x - button_width - button_h_gap,
		.y = center_pos_y,
	});
	next_button_.set_position({
		.x = center_pos_x + button_h_gap,
		.y = center_pos_y,
	});
	reset_button_.set_position({
		.x = center_pos_x + button_h_gap,
		.y = center_pos_y,
	});

	const auto solved = current_puzzle_.is_solved();
	next_button_.set_visible(solved);
	reset_button_.set_visible(!solved);

	return true;
}

auto game::setup_puzzle(const std::string &puzzle_str) -> pxe::result<> {
	if(const auto error = puzzle::from_string(puzzle_str).unwrap(current_puzzle_); error) {
		return pxe::error("failed to parse puzzle from string: {}", *error);
	}

	auto const total_batteries = current_puzzle_.size();
	toggle_batteries(total_batteries);

	for(const auto index: std::views::iota(0, max_batteries)) {
		battery_displays_.at(index).reset();
	}

	for(auto i = static_cast<size_t>(0); i < total_batteries; ++i) {
		batteries_.at(i) = current_puzzle_.at(i);
	}

	for(auto &battery_display: battery_displays_) {
		battery_display.set_enabled(true);
	}

	return true;
}

auto game::show() -> pxe::result<> {
	const auto &app = dynamic_cast<energy_swap &>(get_app());

	title_.set_text(std::format("Level {}", app.get_current_level()));
	title_.set_font_size(30);
	title_.set_centered(true);

	status_.set_text("");
	status_.set_centered(true);

	if(const auto err = scene::show().unwrap(); err) {
		return pxe::error("failed to enable base scene", *err);
	}

	if(const auto err = get_app().play_music(game_music, 0.5F).unwrap(); err) {
		return pxe::error("fail to play game music", *err);
	}

	const auto &level_str = app.get_current_level_string();

	SPDLOG_DEBUG("setting up puzzle with level string: {}", level_str);

	if(const auto err = setup_puzzle(level_str).unwrap(); err) {
		return pxe::error("failed to setup puzzle", *err);
	}

	return true;
}

auto game::reset() -> pxe::result<> {
	return show();
}

auto game::toggle_batteries(const size_t number) -> void {
	auto index = static_cast<size_t>(0);
	for(const auto battery_num: battery_order) {
		battery_displays_.at(index).set_visible(battery_num < number);
		++index;
	}
}

auto game::on_battery_click(const battery_display::click &click) -> pxe::result<> {
	const auto clicked_index = battery_order.at(click.index);

	// find if we have a selected battery
	//	clang-tidy, only in WSL2, complains that we should use auto *const, when this is not a pointer
	const auto selected_it = // NOLINT(*-qualified-auto)
		std::ranges::find_if(battery_displays_, [](const battery_display &battery_display) -> bool {
			return battery_display.is_selected();
		});

	// if we have none
	if(selected_it == battery_displays_.end()) {
		// play click sound only for selecting
		if(const auto err = get_app().play_sound(battery_click_sound).unwrap(); err) {
			return pxe::error("failed to play battery click sound", *err);
		}

		// select the clicked battery if it's not closed or empty
		if(!batteries_.at(clicked_index).closed() && !batteries_.at(clicked_index).empty()) {
			battery_displays_.at(click.index).set_selected(true);
		}
		return true;
	}

	// deselect it
	selected_it->set_selected(false);

	auto need_click_sound = true;

	const auto selected_index = battery_order.at(selected_it->get_index());
	auto &from_battery = batteries_.at(selected_index);

	if(auto &to_battery = batteries_.at(clicked_index);
	   (selected_index != clicked_index) && to_battery.can_get_from(from_battery)) {
		// sparks
		if(const auto err = shoot_sparks(selected_it->get_position(),
										 battery_displays_.at(click.index).get_position(),
										 selected_it->get_top_color(),
										 5)
								.unwrap();
		   err) {
			return pxe::error("failed to shoot sparks", *err);
		}
		// no click sound on successful transfer since we have sparks sound
		need_click_sound = false;

		// transfer energy
		to_battery.transfer_energy_from(from_battery);
		// update puzzle state
		current_puzzle_.at(selected_index) = from_battery;
		current_puzzle_.at(clicked_index) = to_battery;
		check_end();
	}

	if(need_click_sound) {
		if(const auto err = get_app().play_sound(battery_click_sound).unwrap(); err) {
			return pxe::error("failed to play battery click sound", *err);
		}
	}
	return true;
}

auto game::on_button_click(const pxe::button::click &evt) -> pxe::result<> {
	if(evt.id == next_button_.get_id()) {
		get_app().post_event(next_level{});
	} else if(evt.id == back_button_.get_id()) {
		get_app().post_event(back{});
	} else if(evt.id == reset_button_.get_id()) {
		get_app().post_event(reset_level{});
	}

	return true;
}

auto game::check_end() -> void {
	auto game_ended = false;
	if(current_puzzle_.is_solved()) {
		status_.set_text("You Win, continue to the next level ...");
		next_button_.set_visible(true);
		reset_button_.set_visible(false);
		game_ended = true;
	} else if(!current_puzzle_.is_solvable()) {
		status_.set_text("No more moves available, try again ...");
		game_ended = true;
	}

	if(game_ended) {
		for(auto &battery_display: battery_displays_) {
			battery_display.set_enabled(false);
		}
	}
}

auto game::find_free_spark() -> std::shared_ptr<spark> {
	for(const auto &id: sparks_) {
		std::shared_ptr<spark> spark_ptr;
		if(const auto err = get_component<spark>(id).unwrap(spark_ptr); err) {
			continue;
		}
		if(!spark_ptr->is_visible()) {
			return spark_ptr;
		}
	}
	return nullptr;
}

auto game::shoot_sparks(const Vector2 from, const Vector2 to, const Color color, const size_t count) -> pxe::result<> {
	if(const auto err = get_app().play_sound(zap_sound).unwrap(); err) {
		return pxe::error("failed to play zap sound", *err);
	}

	for(size_t i = 0; i < count; ++i) {
		// from and to will be slightly random in each spark animation
		auto new_from = Vector2{
			.x = from.x + static_cast<float>(GetRandomValue(-10, 10)),
			.y = from.y + static_cast<float>(GetRandomValue(-10, 10)),
		};
		auto new_to = Vector2{
			.x = to.x + static_cast<float>(GetRandomValue(-10, 10)),
			.y = to.y + static_cast<float>(GetRandomValue(-10, 10)),
		};

		if(const auto spark = find_free_spark(); spark != nullptr) {
			spark->set_tint(color);
			spark->set_position(new_from);
			spark->set_destination(new_to);
			spark->set_visible(true);
			spark->play();
		} else {
			SPDLOG_WARN("no free spark found to play animation");
		}
	}
	return true;
}

} // namespace energy