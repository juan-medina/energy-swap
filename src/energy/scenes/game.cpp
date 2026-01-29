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
#include "../data/puzzle.hpp"
#include "../energy_swap.hpp"
#include "../level_manager.hpp"

#include <raylib.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <format>
#include <limits>
#include <memory>
#include <optional>
#include <ranges>
#include <raygui.h>
#include <spdlog/spdlog.h>
#include <vector>

namespace energy {

// ============================================================================
// Lifecycle
// ============================================================================

auto game::init(pxe::app &app) -> pxe::result<> {
	if(const auto err = scene::init(app).unwrap(); err) {
		return pxe::error("failed to initialize base component", *err);
	}

	SPDLOG_INFO("game scene initialized");

	if(const auto err = init_ui_components().unwrap(); err) {
		return pxe::error("failed to initialize UI components", *err);
	}

	if(const auto err = init_battery_displays().unwrap(); err) {
		return pxe::error("failed to initialize battery displays", *err);
	}

	if(const auto err = init_buttons().unwrap(); err) {
		return pxe::error("failed to initialize buttons", *err);
	}

	if(const auto err = init_sparks().unwrap(); err) {
		return pxe::error("failed to initialize sparks", *err);
	}

	battery_click_ = app.bind_event<battery_display::click>(this, &game::on_battery_click);
	button_click_ = app.bind_event<pxe::button::click>(this, &game::on_button_click);

	return true;
}

auto game::end() -> pxe::result<> {
	get_app().unsubscribe(button_click_);
	get_app().unsubscribe(battery_click_);

	return scene::end();
}

auto game::show() -> pxe::result<> {
	if(const auto err = configure_show_ui().unwrap(); err) {
		return pxe::error("failed to configure UI", *err);
	}

	if(const auto err = configure_button_visibility().unwrap(); err) {
		return pxe::error("failed to configure button visibility", *err);
	}

	if(const auto err = scene::show().unwrap(); err) {
		return pxe::error("failed to enable base scene", *err);
	}

	if(const auto err = get_app().play_music(game_music).unwrap(); err) {
		return pxe::error("fail to play game music", *err);
	}

	auto &app = dynamic_cast<energy_swap &>(get_app());
	std::string level_str;
	if(const auto err = app.get_level_manager().get_current_level_string().unwrap(level_str); err) {
		return pxe::error("failed to get current level string", *err);
	}
	can_have_solution_hint_ = app.get_level_manager().can_have_solution_hint();

	SPDLOG_DEBUG("setting up puzzle with level string: {}", level_str);

	if(const auto err = setup_puzzle(level_str).unwrap(); err) {
		return pxe::error("failed to setup puzzle", *err);
	}

	return true;
}

auto game::reset() -> pxe::result<> {
	for(const auto &spark: get_components_of_type<spark>()) {
		spark->set_visible(false);
	}

	for(const auto &battery: get_components_of_type<battery_display>()) {
		battery->reset(); // NOLINT(*-ambiguous-smartptr-reset-call)
	}

	return show();
}

auto game::update(const float delta) -> pxe::result<> {
	if(!is_enabled() || !is_visible()) {
		return true;
	}

	if(const auto err = scene::update(delta).unwrap(); err) {
		return pxe::error("failed to update base scene", *err);
	}

	if(get_app().is_in_controller_mode()) {
		if(const auto err = update_controller_input().unwrap(); err) {
			return pxe::error("failed to update controller input", *err);
		}
	}

	if(is_cosmic_level_ && !time_paused_) {
		remaining_time_ -= delta;
		auto const seconds_str = std::format("{:.2f}", std::max(0.0F, remaining_time_));
		std::shared_ptr<pxe::label> time_ptr;
		if(const auto err = get_component<pxe::label>(time_).unwrap(time_ptr); err) {
			return pxe::error("failed to get time label", *err);
		}
		time_ptr->set_text(seconds_str);
		if(remaining_time_ <= 0.0F) {
			if(const auto err = handle_cosmic_time_up().unwrap(); err) {
				return pxe::error("failed to handle cosmic time up", *err);
			}
		} else if(remaining_time_ < 10.0F) {
			time_ptr->set_text_color(RED);
		} else if(remaining_time_ <= 30.0F) {
			time_ptr->set_text_color(YELLOW);
		} else {
			time_ptr->set_text_color(GREEN);
		}
	}

	return true;
}

// ============================================================================
// Initialization
// ============================================================================

auto game::init_ui_components() -> pxe::result<> {
	if(const auto err = register_component<pxe::label>().unwrap(title_); err) {
		return pxe::error("failed to register title label", *err);
	}

	if(const auto err = register_component<pxe::label>().unwrap(status_); err) {
		return pxe::error("failed to register status label", *err);
	}

	if(const auto err = register_component<pxe::label>().unwrap(time_); err) {
		return pxe::error("failed to register time label", *err);
	}

	std::shared_ptr<pxe::label> title_ptr;
	if(const auto err = get_component<pxe::label>(title_).unwrap(title_ptr); err) {
		return pxe::error("failed to get title label", *err);
	}
	title_ptr->set_font_size(30);
	title_ptr->set_text("Level 000");

	return true;
}

auto game::init_battery_displays() -> pxe::result<> {
	size_t id{0};
	for(const auto counter: std::views::iota(0, max_batteries)) {
		if(const auto err = register_component<battery_display>().unwrap(id); err) {
			return pxe::error("failed to register battery display", *err);
		}
		std::shared_ptr<battery_display> battery_display_ptr;
		if(const auto err = get_battery_display(id).unwrap(battery_display_ptr); err) {
			return pxe::error("failed to get battery display component", *err);
		}
		battery_display_ptr->set_visible(false);
		battery_display_ptr->set_index(battery_order.at(counter)); // set the battery order index
	}

	return true;
}

auto game::init_buttons() -> pxe::result<> {
	if(const auto err = register_component<pxe::button>().unwrap(back_button_); err) {
		return pxe::error("failed to register back button", *err);
	}

	if(const auto err = register_component<pxe::button>().unwrap(next_button_); err) {
		return pxe::error("failed to register next button", *err);
	}

	if(const auto err = register_component<pxe::button>().unwrap(reset_button_); err) {
		return pxe::error("failed to register reset button", *err);
	}

	std::shared_ptr<pxe::button> back_button_ptr;
	if(const auto err = get_component<pxe::button>(back_button_).unwrap(back_button_ptr); err) {
		return pxe::error("failed to get back button", *err);
	}

	std::shared_ptr<pxe::button> next_button_ptr;
	if(const auto err = get_component<pxe::button>(next_button_).unwrap(next_button_ptr); err) {
		return pxe::error("failed to get next button", *err);
	}

	std::shared_ptr<pxe::button> reset_button_ptr;
	if(const auto err = get_component<pxe::button>(reset_button_).unwrap(reset_button_ptr); err) {
		return pxe::error("failed to get reset button", *err);
	}

	back_button_ptr->set_text(GuiIconText(ICON_PLAYER_PREVIOUS, "Back"));
	back_button_ptr->set_position({.x = 0, .y = 0});
	back_button_ptr->set_size({.width = 75, .height = 25});

	next_button_ptr->set_text(GuiIconText(ICON_PLAYER_NEXT, "Next"));
	next_button_ptr->set_position({.x = 0, .y = 0});
	next_button_ptr->set_size({.width = 75, .height = 25});

	reset_button_ptr->set_text(GuiIconText(ICON_UNDO, "Reset"));
	reset_button_ptr->set_position({.x = 0, .y = 0});
	reset_button_ptr->set_size({.width = 75, .height = 25});

	return true;
}

auto game::init_sparks() -> pxe::result<> {
	for([[maybe_unused]] const auto num: std::views::iota(0, max_batteries)) {
		auto id = size_t{0};
		if(const auto err = register_component<spark>().unwrap(id); err) {
			return pxe::error("failed to register spark animation", *err);
		}

		std::shared_ptr<spark> spark_ptr;
		if(const auto err = get_component<spark>(id).unwrap(spark_ptr); err) {
			return pxe::error("failed to get spark animation", *err);
		}
		spark_ptr->set_scale(2.0F);
		spark_ptr->set_visible(false);
	}
	return true;
}

// ============================================================================
// Layout
// ============================================================================

auto game::layout(const pxe::size screen_size) -> pxe::result<> {
	if(const auto err = layout_title(screen_size).unwrap(); err) {
		return pxe::error("failed to layout title", *err);
	}

	if(const auto err = layout_status(screen_size).unwrap(); err) {
		return pxe::error("failed to layout status", *err);
	}

	if(const auto err = layout_batteries(screen_size).unwrap(); err) {
		return pxe::error("failed to layout batteries", *err);
	}

	if(const auto err = layout_buttons(screen_size).unwrap(); err) {
		return pxe::error("failed to layout buttons", *err);
	}

	return true;
}

auto game::layout_title(const pxe::size screen_size) const -> pxe::result<> {
	std::shared_ptr<pxe::label> title_ptr;
	if(const auto err = get_component<pxe::label>(title_).unwrap(title_ptr); err) {
		return pxe::error("failed to get title label", *err);
	}

	title_ptr->set_position({
		.x = screen_size.width / 2.0F,
		.y = 10.0F,
	});

	std::shared_ptr<pxe::label> time_ptr;
	if(const auto err = get_component<pxe::label>(time_).unwrap(time_ptr); err) {
		return pxe::error("failed to get time label", *err);
	}

	time_ptr->set_position({
		.x = screen_size.width / 2.0F,
		.y = 10.0F + title_ptr->get_size().height + 5.0F,
	});

	return true;
}

auto game::layout_status(const pxe::size screen_size) const -> pxe::result<> {
	std::shared_ptr<pxe::label> status_ptr;
	if(const auto err = get_component<pxe::label>(status_).unwrap(status_ptr); err) {
		return pxe::error("failed to get status label", *err);
	}

	status_ptr->set_position({
		.x = screen_size.width / 2.0F,
		.y = screen_size.height - 60.0F,
	});

	return true;
}

auto game::layout_batteries(const pxe::size screen_size) const -> pxe::result<> {
	constexpr int rows = 2;
	constexpr int cols = max_batteries / rows;
	const auto horizontal_space = screen_size.width * 0.8F;
	const auto vertical_space = screen_size.height * 0.7F;
	const auto battery_width = horizontal_space / cols;
	const auto battery_height = vertical_space / rows;
	const auto start_x = (screen_size.width - horizontal_space) / 2.0F;
	const auto start_y = (screen_size.height - vertical_space) / 2.0F;

	size_t i{0};
	for(const auto &battery: get_components_of_type<battery_display>()) {
		auto const row = i / cols;
		auto const col = i % cols;
		auto const pos_x = start_x + (battery_width * static_cast<float>(col)) + (battery_width / 2.0F);
		auto const pos_y = start_y + (battery_height * static_cast<float>(row)) + (battery_height / 2.0F);
		battery->set_position({.x = pos_x, .y = pos_y});
		i++;
	}

	return true;
}

auto game::layout_buttons(const pxe::size screen_size) const -> pxe::result<> {
	std::shared_ptr<pxe::button> back_button_ptr;
	if(const auto err = get_component<pxe::button>(back_button_).unwrap(back_button_ptr); err) {
		return pxe::error("failed to get back button", *err);
	}

	std::shared_ptr<pxe::button> next_button_ptr;
	if(const auto err = get_component<pxe::button>(next_button_).unwrap(next_button_ptr); err) {
		return pxe::error("failed to get next button", *err);
	}

	std::shared_ptr<pxe::button> reset_button_ptr;
	if(const auto err = get_component<pxe::button>(reset_button_).unwrap(reset_button_ptr); err) {
		return pxe::error("failed to get reset button", *err);
	}

	constexpr auto button_v_gap = 10.0F;
	constexpr auto button_h_gap = 10.0F;

	const auto [button_width, button_height] = back_button_ptr->get_size();

	const auto center_pos_x = screen_size.width * 0.5F;
	const auto center_pos_y = screen_size.height - button_height - button_v_gap;

	back_button_ptr->set_position({
		.x = center_pos_x - button_width - button_h_gap,
		.y = center_pos_y,
	});

	next_button_ptr->set_position({
		.x = center_pos_x + button_h_gap,
		.y = center_pos_y,
	});

	reset_button_ptr->set_position({
		.x = center_pos_x + button_h_gap,
		.y = center_pos_y,
	});

	next_button_ptr->set_visible(false);
	reset_button_ptr->set_visible(true);

	return true;
}

// ============================================================================
// Configuration
// ============================================================================

auto game::configure_show_ui() -> pxe::result<> {
	const auto &app = dynamic_cast<energy_swap &>(get_app());

	std::shared_ptr<pxe::label> title_ptr;
	if(const auto err = get_component<pxe::label>(title_).unwrap(title_ptr); err) {
		return pxe::error("failed to get title label", *err);
	}

	std::shared_ptr<pxe::label> time_ptr;
	if(const auto err = get_component<pxe::label>(time_).unwrap(time_ptr); err) {
		return pxe::error("failed to get time label", *err);
	}

	std::shared_ptr<pxe::label> status_ptr;
	if(const auto err = get_component<pxe::label>(status_).unwrap(status_ptr); err) {
		return pxe::error("failed to get status label", *err);
	}

	is_cosmic_level_ = app.get_level_manager().get_mode() == level_manager::mode::cosmic;

	if(!is_cosmic_level_) {
		title_ptr->set_text(std::format(format_classic_level_title, app.get_level_manager().get_current_level()));
	} else {
		switch(app.get_level_manager().get_difficulty()) {
		case level_manager::difficulty::normal:
			title_ptr->set_text(
				std::format(format_cosmic_level_title, cosmic_normal, app.get_level_manager().get_current_level()));
			break;
		case level_manager::difficulty::hard:
			title_ptr->set_text(
				std::format(format_cosmic_level_title, cosmic_hard, app.get_level_manager().get_current_level()));
			break;
		case level_manager::difficulty::burger_daddy:
			title_ptr->set_text(std::format(
				format_cosmic_level_title, cosmic_burger_daddy, app.get_level_manager().get_current_level()));
			break;
		}
	}
	title_ptr->set_font_size(30);
	title_ptr->set_centered(true);

	time_ptr->set_text("Time: 00:00");
	time_ptr->set_font_size(20);
	time_ptr->set_text_color(GREEN);
	time_ptr->set_centered(true);

	if(is_cosmic_level_) {
		time_ptr->set_visible(true);
		remaining_time_ = app.get_time_for_cosmic();
		time_paused_ = false;
	} else {
		time_ptr->set_visible(false);
	}

	status_ptr->set_text("");
	status_ptr->set_centered(true);

	return true;
}

auto game::configure_button_visibility() const -> pxe::result<> {
	std::shared_ptr<pxe::button> back_button_ptr;
	if(const auto err = get_component<pxe::button>(back_button_).unwrap(back_button_ptr); err) {
		return pxe::error("failed to get back button", *err);
	}

	std::shared_ptr<pxe::button> next_button_ptr;
	if(const auto err = get_component<pxe::button>(next_button_).unwrap(next_button_ptr); err) {
		return pxe::error("failed to get next button", *err);
	}

	std::shared_ptr<pxe::button> reset_button_ptr;
	if(const auto err = get_component<pxe::button>(reset_button_).unwrap(reset_button_ptr); err) {
		return pxe::error("failed to get reset button", *err);
	}

	back_button_ptr->set_visible(true);
	back_button_ptr->set_controller_button(GAMEPAD_BUTTON_RIGHT_FACE_RIGHT);
	next_button_ptr->set_visible(false);
	next_button_ptr->set_controller_button(GAMEPAD_BUTTON_RIGHT_FACE_UP);
	reset_button_ptr->set_visible(true);
	reset_button_ptr->set_controller_button(GAMEPAD_BUTTON_RIGHT_FACE_UP);

	return true;
}

// ============================================================================
// Puzzle Setup
// ============================================================================

auto game::setup_puzzle(const std::string &puzzle_str) -> pxe::result<> {
	if(const auto error = puzzle::from_string(puzzle_str).unwrap(current_puzzle_); error) {
		return pxe::error("failed to parse puzzle from string", *error);
	}

	auto const total_batteries = current_puzzle_.size();
	toggle_batteries(total_batteries);

	for(const auto &battery: get_components_of_type<battery_display>()) {
		battery->reset(); // NOLINT(*-ambiguous-smartptr-reset-call)
		battery->set_enabled(true);
	}

	if(const auto err = calculate_solution_hint().unwrap(); err) {
		return pxe::error("failed to calculate solution hint", *err);
	}
	return true;
}

// ============================================================================
// Battery Management
// ============================================================================

auto game::toggle_batteries(const size_t number) -> void {
	for(const auto &battery: get_components_of_type<battery_display>()) {
		const auto index = battery->get_index();
		battery->set_visible(index < number);
		if(index < number) {
			battery->set_battery(current_puzzle_.at(index));
		}
	}
}

auto game::disable_all_batteries() const -> pxe::result<> {
	for(const auto &battery: get_components_of_type<battery_display>()) {
		battery->set_enabled(false);
		battery->set_selected(false);
	}
	if(const auto err = reset_hint_indicators().unwrap(); err) {
		return pxe::error("failed to reset hint indicators", *err);
	}
	return true;
}

auto game::find_selected_battery() const -> std::shared_ptr<battery_display> {
	for(const auto &battery: get_components_of_type<battery_display>()) {
		if(battery->is_selected()) {
			return battery;
		}
	}
	return nullptr;
}

auto game::find_focussed_battery() const -> std::shared_ptr<battery_display> {
	for(const auto &battery: get_components_of_type<battery_display>()) {
		if(battery->is_focussed()) {
			return battery;
		}
	}
	return nullptr;
}

auto game::get_battery_display(const size_t id) const -> pxe::result<std::shared_ptr<battery_display>> {
	std::shared_ptr<battery_display> battery_ptr;
	if(const auto err = get_component<battery_display>(id).unwrap(battery_ptr); err) {
		return pxe::error("failed to get battery display component", *err);
	}
	return battery_ptr;
}

// ============================================================================
// Event Handlers
// ============================================================================

auto game::on_battery_click(const battery_display::click &click) -> pxe::result<> {
	std::shared_ptr<battery_display> clicked_battery;
	if(const auto err = get_battery_display(click.id).unwrap(clicked_battery); err) {
		return pxe::error("failed to get battery display component", *err);
	}

	const auto selected_ptr = find_selected_battery();

	if(selected_ptr == nullptr) {
		if(got_hint_ && can_have_solution_hint_) {
			// if we have a hint, and the clicked battery is the "from" battery, hint the "to" battery
			if(const auto clicked_index = clicked_battery->get_index(); clicked_index == hint_from_) {
				if(const auto err = set_hint_to_battery(hint_from_, false).unwrap(); err) {
					return pxe::error("failed to clear hint to battery", *err);
				}
				if(const auto err = set_hint_to_battery(hint_to_, true).unwrap(); err) {
					return pxe::error("failed to set hint to battery", *err);
				}
			}
		}

		return handle_battery_selection(clicked_battery);
	}

	if(const auto err = handle_battery_transfer(selected_ptr, clicked_battery).unwrap(); err) {
		return pxe::error("failed to handle battery transfer", *err);
	}

	if(got_hint_ && can_have_solution_hint_) {
		// if we handle a transfer get a hint, this includes when we deselect a battery, or we need the next hint
		if(const auto err = calculate_solution_hint().unwrap(); err) {
			return pxe::error("failed to calculate solution hint", *err);
		}
	}

	return true;
}

auto game::on_button_click(const pxe::button::click &evt) -> pxe::result<> {
	if(evt.id == next_button_) {
		get_app().post_event(next_level{});
	} else if(evt.id == back_button_) {
		get_app().post_event(back{});
	} else if(evt.id == reset_button_) {
		get_app().post_event(reset_level{});
	}

	return true;
}

// ============================================================================
// Battery Click Processing
// ============================================================================

auto game::handle_battery_selection(const std::shared_ptr<battery_display> &clicked) -> pxe::result<> {
	if(const auto err = get_app().play_sfx(battery_click_sound).unwrap(); err) {
		return pxe::error("failed to play battery click sound", *err);
	}

	if(!clicked->is_battery_closed() && !clicked->is_battery_empty()) {
		clicked->set_selected(true);
	}

	return true;
}

auto game::handle_battery_transfer(const std::shared_ptr<battery_display> &selected,
								   const std::shared_ptr<battery_display> &clicked) -> pxe::result<> {
	selected->set_selected(false);

	auto need_click_sound = true;

	if((selected->get_id() != clicked->get_id()) && clicked->can_get_from(*selected)) {
		if(const auto err = execute_energy_transfer(selected, clicked).unwrap(); err) {
			return pxe::error("failed to execute energy transfer", *err);
		}

		need_click_sound = false;
	}

	if(need_click_sound) {
		if(const auto err = get_app().play_sfx(battery_click_sound).unwrap(); err) {
			return pxe::error("failed to play battery click sound", *err);
		}
	}

	return true;
}

// ============================================================================
// Win/Lose Conditions
// ============================================================================

auto game::check_end() -> pxe::result<> {
	if(current_puzzle_.is_solved()) {
		return handle_puzzle_solved();
	}

	if(!current_puzzle_.is_solvable()) {
		return handle_puzzle_unsolvable();
	}

	return true;
}

auto game::handle_puzzle_solved() -> pxe::result<> {
	time_paused_ = true;
	auto &app = dynamic_cast<energy_swap &>(get_app());
	app.set_time_for_cosmic(remaining_time_);
	const auto current_level = app.get_level_manager().get_current_level();
	const auto total_levels = app.get_level_manager().get_total_levels();

	if(current_level >= total_levels) {
		if(const auto err = update_end_game_ui(win_message, false, false).unwrap(); err) {
			return pxe::error("failed to update end game UI", *err);
		}
	} else {
		if(const auto err = update_end_game_ui(continue_message, true, false).unwrap(); err) {
			return pxe::error("failed to update end game UI", *err);
		}
	}

	if(const auto err = disable_all_batteries().unwrap(); err) {
		return pxe::error("failed to disable all batteries", *err);
	}

	return true;
}

auto game::handle_puzzle_unsolvable() const -> pxe::result<> {
	if(const auto err = update_end_game_ui(unsolvable_message, false, true).unwrap(); err) {
		return pxe::error("failed to update end game UI", *err);
	}

	if(const auto err = disable_all_batteries().unwrap(); err) {
		return pxe::error("failed to disable all batteries", *err);
	}

	return true;
}

auto game::handle_cosmic_time_up() -> pxe::result<> {
	time_paused_ = true;
	if(const auto err = update_end_game_ui(cosmic_time_up_message, false, true).unwrap(); err) {
		return pxe::error("failed to update end game UI", *err);
	}

	if(const auto err = disable_all_batteries().unwrap(); err) {
		return pxe::error("failed to disable all batteries", *err);
	}

	return true;
}

auto game::update_end_game_ui(const std::string &status_message, const bool show_next, const bool show_reset) const
	-> pxe::result<> {
	std::shared_ptr<pxe::label> status_ptr;
	if(const auto err = get_component<pxe::label>(status_).unwrap(status_ptr); err) {
		return pxe::error("failed to get status label", *err);
	}

	std::shared_ptr<pxe::button> next_button_ptr;
	if(const auto err = get_component<pxe::button>(next_button_).unwrap(next_button_ptr); err) {
		return pxe::error("failed to get next button", *err);
	}

	std::shared_ptr<pxe::button> reset_button_ptr;
	if(const auto err = get_component<pxe::button>(reset_button_).unwrap(reset_button_ptr); err) {
		return pxe::error("failed to get reset button", *err);
	}

	status_ptr->set_text(status_message);
	next_button_ptr->set_visible(show_next);
	reset_button_ptr->set_visible(show_reset);

	return true;
}

// ============================================================================
// Visual Effects
// ============================================================================

auto game::shoot_sparks(const Vector2 from, const Vector2 to, const Color color, const size_t count) -> pxe::result<> {
	if(const auto err = get_app().play_sfx(zap_sound).unwrap(); err) {
		return pxe::error("failed to play zap sound", *err);
	}

	for(size_t i = 0; i < count; ++i) {
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
			SPDLOG_WARN("no free spark found to shoot");
		}
	}
	return true;
}

auto game::find_free_spark() const -> std::shared_ptr<spark> {
	for(const auto &spark: get_components_of_type<spark>()) {
		if(!spark->is_visible()) {
			return spark;
		}
	}
	return nullptr;
}

// ============================================================================
// Controller Input
// ============================================================================

auto game::update_controller_input() -> pxe::result<> {
	auto const focused = find_focussed_battery();
	if(focused == nullptr) {
		if(should_auto_focus_battery()) {
			if(const auto err = auto_focus_first_available_battery().unwrap(); err) {
				return pxe::error("failed to auto focus battery", *err);
			}
		}
		return true;
	}

	if(const auto err = controller_move_battery(focused).unwrap(); err) {
		return pxe::error("failed to handle controller battery move", *err);
	}

	return true;
}

auto game::should_auto_focus_battery() const -> bool {
	return !current_puzzle_.is_solved() && current_puzzle_.is_solvable();
}

auto game::auto_focus_first_available_battery() const -> pxe::result<> {
	for(const auto &battery: get_components_of_type<battery_display>()) {
		if(battery->is_visible() && !battery->is_battery_closed()) {
			battery->set_focussed(true);
			return true;
		}
	}
	return true;
}

auto game::controller_move_battery(const std::shared_ptr<battery_display> &focus) -> pxe::result<> {
	if(!is_enabled()) {
		return true;
	}

	const auto &app = get_app();
	const auto left = app.is_direction_pressed(pxe::direction::left);
	const auto right = app.is_direction_pressed(pxe::direction::right);
	const auto up = app.is_direction_pressed(pxe::direction::up);
	const auto down = app.is_direction_pressed(pxe::direction::down);

	if(left || right || up || down) {
		const auto dx = left ? -1 : (right ? 1 : 0); // NOLINT(*-avoid-nested-conditional-operator)
		const auto dy = up ? -1 : (down ? 1 : 0);	 // NOLINT(*-avoid-nested-conditional-operator)
		return move_focus_to(focus, dx, dy);
	}

	return true;
}

auto game::move_focus_to(const std::shared_ptr<battery_display> &focus, const int dx, const int dy) const
	-> pxe::result<> {
	if(focus == nullptr) {
		return true;
	}

	if(const auto closest = find_closest_battery_in_direction(focus, dx, dy); closest != nullptr) {
		focus->set_focussed(false);
		closest->set_focussed(true);
	}

	return true;
}

auto game::find_closest_battery_in_direction(const std::shared_ptr<battery_display> &focus,
											 const int dx,
											 const int dy) const -> std::shared_ptr<battery_display> {
	if(focus == nullptr) {
		return nullptr;
	}

	const auto focus_pos = focus->get_position();
	std::shared_ptr<battery_display> closest_ptr;
	auto closest_distance = std::numeric_limits<float>::max();

	for(const auto &battery: get_components_of_type<battery_display>()) {
		if(battery->get_id() == focus->get_id()) {
			continue;
		}

		if(!battery->is_visible()) {
			continue;
		}

		if(battery->is_battery_closed()) {
			continue;
		}

		const auto battery_pos = battery->get_position();

		if(!is_battery_in_direction(focus_pos, battery_pos, dx, dy)) {
			continue;
		}

		const auto delta_x = battery_pos.x - focus_pos.x;
		const auto delta_y = battery_pos.y - focus_pos.y;

		if(const auto distance = std::sqrt((delta_x * delta_x) + (delta_y * delta_y)); distance < closest_distance) {
			closest_distance = distance;
			closest_ptr = battery;
		}
	}

	return closest_ptr;
}

auto game::is_battery_in_direction(const Vector2 focus_pos, const Vector2 candidate_pos, const int dx, const int dy)
	-> bool {
	const auto delta_x = candidate_pos.x - focus_pos.x;
	const auto delta_y = candidate_pos.y - focus_pos.y;

	return (dx != 0 && std::signbit(delta_x) == std::signbit(static_cast<float>(dx)) && std::abs(delta_x) > 1.0F)
		   || (dy != 0 && std::signbit(delta_y) == std::signbit(static_cast<float>(dy)) && std::abs(delta_y) > 1.0F);
}

auto game::set_hint_to_battery(const size_t battery_num, const bool is_hint) const -> pxe::result<> {
	for(const auto &battery: get_components_of_type<battery_display>()) {
		if(battery->get_index() == battery_num) {
			battery->set_hint(is_hint);
			return true;
		}
	}
	return pxe::error("failed to find battery to set hint");
}

auto game::calculate_solution_hint() -> pxe::result<> {
	if(!can_have_solution_hint_) {
		return true;
	}

	got_hint_ = false;
	if(current_puzzle_.is_solved()) {
		return true;
	}
	if(const auto solution_moves = current_puzzle_.solve(); !solution_moves.empty()) {
		const auto [from, to] = solution_moves.front();
		hint_from_ = from;
		hint_to_ = to;
		got_hint_ = true;
		if(const auto err = reset_hint_indicators().unwrap(); err) {
			return pxe::error("failed to reset hint indicators", *err);
		}
		if(const auto err = set_hint_to_battery(from, true).unwrap(); err) {
			return pxe::error("failed to set hint to battery", *err);
		}
		return true;
	}
	return pxe::error("no solution found for current puzzle state");
}

auto game::reset_hint_indicators() const -> pxe::result<> {
	for(const auto &battery: get_components_of_type<battery_display>()) {
		battery->set_hint(false);
	}
	return true;
}

auto game::execute_energy_transfer(const std::shared_ptr<battery_display> &from,
								   const std::shared_ptr<battery_display> &to) -> pxe::result<> {
	if(const auto err = shoot_sparks(from->get_position(), to->get_position(), from->get_top_color(), 5).unwrap();
	   err) {
		return pxe::error("failed to shoot sparks", *err);
	}

	to->transfer_energy_from(*from);

	if(is_cosmic_level_) {
		if(to->is_battery_closed()) {
			remaining_time_ += 5;
		}
	}

	if(const auto err = check_end().unwrap(); err) {
		return pxe::error("failed to check end condition", *err);
	}

	return true;
}

} // namespace energy
