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

#include <raylib.h>

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

namespace energy {

auto game::init(pxe::app &app) -> pxe::result<> {
	if(const auto err = scene::init(app).unwrap(); err) {
		return pxe::error("failed to initialize base component", *err);
	}

	SPDLOG_INFO("game scene initialized");

	if(const auto err = init_ui_components().unwrap(); err) {
		return pxe::error("failed to initialize UI components", *err);
	}

	if(const auto err = app.load_sprite_sheet(sprite_sheet_name, sprite_sheet_path).unwrap(); err) {
		return pxe::error("failed to initialize sprite sheet", *err);
	}

	if(const auto err = init_battery_displays().unwrap(); err) {
		return pxe::error("failed to initialize battery displays", *err);
	}

	if(const auto err = init_buttons().unwrap(); err) {
		return pxe::error("failed to initialize buttons", *err);
	}

	battery_click_ = app.bind_event<battery_display::click>(this, &game::on_battery_click);
	button_click_ = app.bind_event<pxe::button::click>(this, &game::on_button_click);

	if(const auto err = init_sparks().unwrap(); err) {
		return pxe::error("failed to initialize sparks", *err);
	}

	return true;
}

auto game::end() -> pxe::result<> {
	if(const auto err = get_app().unload_sprite_sheet(sprite_sheet_name).unwrap(); err) {
		return pxe::error("failed to end sprite sheet", *err);
	}

	get_app().unsubscribe(button_click_);
	get_app().unsubscribe(battery_click_);

	return scene::end();
}

auto game::update(const float delta) -> pxe::result<> {
	if(const auto err = scene::update(delta).unwrap(); err) {
		return pxe::error("failed to update base scene", *err);
	}

	if(get_app().is_in_controller_mode()) {
		auto const focused = find_focussed_battery();
		if(!focused.has_value()) {
			if(current_puzzle_.is_solved() || !current_puzzle_.is_solvable()) {
				return true;
			}
			// find first visible battery that is not locked or full
			for(const auto id: battery_displays_) {
				std::shared_ptr<battery_display> battery_ptr;
				if(const auto err = get_component<battery_display>(id).unwrap(battery_ptr); err) {
					return pxe::error("failed to get battery display component", *err);
				}

				const auto idx = battery_ptr->get_index();
				if(auto &bat = current_puzzle_.at(idx); battery_ptr->is_visible() && !bat.closed()) {
					battery_ptr->set_focussed(true);
					return true;
				}
			}
		}
		if(const auto err = controller_move_battery(*focused).unwrap(); err) {
			return pxe::error("failed to handle controller battery move", *err);
		}
	}

	return true;
}

auto game::init_ui_components() -> pxe::result<> {
	if(const auto err = register_component<pxe::label>().unwrap(title_); err) {
		return pxe::error("failed to register title label", *err);
	}

	if(const auto err = register_component<pxe::label>().unwrap(status_); err) {
		return pxe::error("failed to register status label", *err);
	}

	return true;
}

auto game::init_battery_displays() -> pxe::result<> {
	for(auto &id: battery_displays_) {
		if(const auto err = register_component<battery_display>().unwrap(id); err) {
			return pxe::error("failed to register battery display", *err);
		}
		std::shared_ptr<battery_display> battery_display_ptr;
		if(const auto err = get_component<battery_display>(id).unwrap(battery_display_ptr); err) {
			return pxe::error("failed to get battery display component", *err);
		}
		battery_display_ptr->set_visible(false);
	}

	for(const auto index: std::views::iota(0, max_batteries)) {
		const auto id = battery_displays_.at(index);
		std::shared_ptr<battery_display> battery_display_ptr;
		if(const auto err = get_component<battery_display>(id).unwrap(battery_display_ptr); err) {
			return pxe::error("failed to get battery display component", *err);
		}
		battery_display_ptr->set_index(index);
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
	back_button_ptr->set_size({.width = 65, .height = 25});

	next_button_ptr->set_text(GuiIconText(ICON_PLAYER_NEXT, "Next"));
	next_button_ptr->set_position({.x = 0, .y = 0});
	next_button_ptr->set_size({.width = 65, .height = 25});

	reset_button_ptr->set_text(GuiIconText(ICON_UNDO, "Reset"));
	reset_button_ptr->set_position({.x = 0, .y = 0});
	reset_button_ptr->set_size({.width = 65, .height = 25});

	return true;
}

auto game::init_sparks() -> pxe::result<> {
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

	for(int i = 0; i < max_batteries; ++i) {
		auto const row = i / cols;
		auto const col = i % cols;
		auto const pos_x = start_x + (battery_width * static_cast<float>(col)) + (battery_width / 2.0F);
		auto const pos_y = start_y + (battery_height * static_cast<float>(row)) + (battery_height / 2.0F);
		const auto id = battery_displays_.at(i);
		std::shared_ptr<battery_display> battery_ptr;
		if(const auto err = get_component<battery_display>(id).unwrap(battery_ptr); err) {
			return pxe::error("failed to get battery display component", *err);
		}
		battery_ptr->set_position({.x = pos_x, .y = pos_y});
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

auto game::setup_puzzle(const std::string &puzzle_str) -> pxe::result<> {
	if(const auto error = puzzle::from_string(puzzle_str).unwrap(current_puzzle_); error) {
		return pxe::error("failed to parse puzzle from string", *error);
	}

	auto const total_batteries = current_puzzle_.size();
	toggle_batteries(total_batteries);

	for(const auto index: std::views::iota(0, max_batteries)) {
		const auto id = battery_displays_.at(index);
		std::shared_ptr<battery_display> battery_ptr;
		if(const auto err = get_component<battery_display>(id).unwrap(battery_ptr); err) {
			return pxe::error("failed to get battery display component", *err);
		}
		battery_ptr->reset(); // NOLINT(*-ambiguous-smartptr-reset-call)
	}

	for(const auto &id: battery_displays_) {
		std::shared_ptr<battery_display> battery_ptr;
		if(const auto err = get_component<battery_display>(id).unwrap(battery_ptr); err) {
			return pxe::error("failed to get battery display component", *err);
		}
		battery_ptr->set_enabled(true);
	}

	return true;
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

	const auto &app = dynamic_cast<energy_swap &>(get_app());
	const auto &level_str = app.get_current_level_string();

	SPDLOG_DEBUG("setting up puzzle with level string: {}", level_str);

	if(const auto err = setup_puzzle(level_str).unwrap(); err) {
		return pxe::error("failed to setup puzzle", *err);
	}

	return true;
}

auto game::reset() -> pxe::result<> {
	for(const auto &id: sparks_) {
		std::shared_ptr<spark> spark_ptr;
		if(const auto err = get_component<spark>(id).unwrap(spark_ptr); err) {
			return pxe::error("failed to get spark component", *err);
		}
		spark_ptr->set_visible(false);
	}
	for(const auto &id: battery_displays_) {
		std::shared_ptr<battery_display> battery_ptr;
		if(const auto err = get_component<battery_display>(id).unwrap(battery_ptr); err) {
			return pxe::error("failed to get battery display component", *err);
		}
		battery_ptr->reset(); // NOLINT(*-ambiguous-smartptr-reset-call)
	}
	return show();
}

auto game::configure_show_ui() -> pxe::result<> {
	const auto &app = dynamic_cast<energy_swap &>(get_app());

	std::shared_ptr<pxe::label> title_ptr;
	if(const auto err = get_component<pxe::label>(title_).unwrap(title_ptr); err) {
		return pxe::error("failed to get title label", *err);
	}

	std::shared_ptr<pxe::label> status_ptr;
	if(const auto err = get_component<pxe::label>(status_).unwrap(status_ptr); err) {
		return pxe::error("failed to get status label", *err);
	}

	title_ptr->set_text(std::format("Level {}", app.get_current_level()));
	title_ptr->set_font_size(30);
	title_ptr->set_centered(true);

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

auto game::toggle_batteries(const size_t number) -> void {
	auto index = static_cast<size_t>(0);
	for(const auto battery_num: battery_order) {
		const auto id = battery_displays_.at(index);
		std::shared_ptr<battery_display> battery_ptr;
		if(const auto err = get_component<battery_display>(id).unwrap(battery_ptr); err) {
			continue;
		}
		battery_ptr->set_visible(battery_num < number);
		if(battery_num < number) {
			battery_ptr->set_battery(current_puzzle_.at(battery_num));
		}
		++index;
	}
}

auto game::disable_all_batteries() const -> void {
	for(const auto &id: battery_displays_) {
		std::shared_ptr<battery_display> battery_ptr;
		if(const auto err = get_component<battery_display>(id).unwrap(battery_ptr); err) {
			continue;
		}
		battery_ptr->set_enabled(false);
		battery_ptr->set_focussed(false);
	}
}

auto game::on_battery_click(const battery_display::click &click) -> pxe::result<> {
	const auto clicked_index = battery_order.at(click.index);

	const auto click_index_id = battery_displays_.at(click.index);
	std::shared_ptr<battery_display> click_index_battery_ptr;
	if(const auto err = get_component<battery_display>(click_index_id).unwrap(click_index_battery_ptr); err) {
		return pxe::error("failed to get battery display component", *err);
	}

	const auto selected_it = find_selected_battery();

	if(!selected_it.has_value()) {
		return handle_battery_selection(clicked_index, *click_index_battery_ptr);
	}

	return handle_battery_transfer(*selected_it, clicked_index, *click_index_battery_ptr);
}

auto game::find_selected_battery() const -> std::optional<size_t> {
	for(size_t i = 0; i < battery_displays_.size(); ++i) {
		const auto id = battery_displays_.at(i);
		std::shared_ptr<battery_display> battery_ptr;
		if(const auto err = get_component<battery_display>(id).unwrap(battery_ptr); err) {
			continue;
		}
		if(battery_ptr->is_selected()) {
			return i;
		}
	}
	return std::nullopt;
}

auto game::handle_battery_selection(const size_t clicked_index, battery_display &clicked_display) -> pxe::result<> {
	if(const auto err = get_app().play_sfx(battery_click_sound).unwrap(); err) {
		return pxe::error("failed to play battery click sound", *err);
	}

	if(!current_puzzle_.at(clicked_index).closed() && !current_puzzle_.at(clicked_index).empty()) {
		clicked_display.set_selected(true);
	}

	return true;
}

auto game::handle_battery_transfer(const size_t selected_index,
								   const size_t clicked_index,
								   const battery_display &clicked_display) -> pxe::result<> {
	const auto selected_id = battery_displays_.at(selected_index);
	std::shared_ptr<battery_display> selected_battery_ptr;
	if(const auto err = get_component<battery_display>(selected_id).unwrap(selected_battery_ptr); err) {
		return pxe::error("failed to get selected battery display", *err);
	}

	selected_battery_ptr->set_selected(false);

	const auto selected_battery_index = battery_order.at(selected_battery_ptr->get_index());
	auto &from_battery = current_puzzle_.at(selected_battery_index);

	auto need_click_sound = true;

	if(auto &to_battery = current_puzzle_.at(clicked_index);
	   (selected_battery_index != clicked_index) && to_battery.can_get_from(from_battery)) {
		if(const auto err = shoot_sparks(selected_battery_ptr->get_position(),
										 clicked_display.get_position(),
										 selected_battery_ptr->get_top_color(),
										 5)
								.unwrap();
		   err) {
			return pxe::error("failed to shoot sparks", *err);
		}

		need_click_sound = false;

		to_battery.transfer_energy_from(from_battery);
		current_puzzle_.at(selected_battery_index) = from_battery;
		current_puzzle_.at(clicked_index) = to_battery;

		if(const auto err = check_end().unwrap(); err) {
			return pxe::error("failed to check end condition", *err);
		}
	}

	if(need_click_sound) {
		if(const auto err = get_app().play_sfx(battery_click_sound).unwrap(); err) {
			return pxe::error("failed to play battery click sound", *err);
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
	auto &app = dynamic_cast<energy_swap &>(get_app());
	const auto current_level = app.get_current_level();
	const auto total_levels = app.get_total_levels();

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

	if(current_level >= total_levels) {
		status_ptr->set_text("Congratulations! You completed all levels!");
		next_button_ptr->set_visible(false);
		reset_button_ptr->set_visible(false);
	} else {
		status_ptr->set_text("You Win, continue to the next level ...");
		next_button_ptr->set_visible(true);
		reset_button_ptr->set_visible(false);
	}

	disable_all_batteries();

	return true;
}

auto game::handle_puzzle_unsolvable() const -> pxe::result<> {
	std::shared_ptr<pxe::label> status_ptr;
	if(const auto err = get_component<pxe::label>(status_).unwrap(status_ptr); err) {
		return pxe::error("failed to get status label", *err);
	}

	status_ptr->set_text("No more moves available, try again ...");

	disable_all_batteries();

	return true;
}

auto game::find_free_spark() const -> std::shared_ptr<spark> {
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

auto game::find_focussed_battery() const -> std::optional<size_t> {
	for(const auto id: battery_displays_) {
		std::shared_ptr<battery_display> battery_ptr;
		if(const auto err = get_component<battery_display>(id).unwrap(battery_ptr); err) {
			continue;
		}
		if(battery_ptr->is_focussed()) {
			return id;
		}
	}
	return std::nullopt;
}

auto game::controller_move_battery(size_t focused) -> pxe::result<> {
	const auto &app = get_app();
	const auto left = app.is_direction_pressed(pxe::direction::left);
	const auto right = app.is_direction_pressed(pxe::direction::right);
	const auto up = app.is_direction_pressed(pxe::direction::up);
	const auto down = app.is_direction_pressed(pxe::direction::down);

	if(left || right || up || down) {
		const auto dx = left ? -1 : (right ? 1 : 0); // NOLINT(*-avoid-nested-conditional-operator)
		const auto dy = up ? -1 : (down ? 1 : 0);	 // NOLINT(*-avoid-nested-conditional-operator)
		return move_focus_to(focused, dx, dy);
	}
	return true;
}

auto game::move_focus_to(const size_t focus, const int dx, const int dy) -> pxe::result<> {
	std::shared_ptr<battery_display> focus_battery_ptr;
	if(const auto err = get_component<battery_display>(focus).unwrap(focus_battery_ptr); err) {
		return pxe::error("failed to get focussed battery display", *err);
	}

	const auto focus_pos = focus_battery_ptr->get_position();
	std::optional<size_t> closest_id;
	auto closest_distance = std::numeric_limits<float>::max();

	for(const auto id: battery_displays_) {
		if(id == focus) {
			continue;
		}

		std::shared_ptr<battery_display> battery_ptr;
		if(const auto err = get_component<battery_display>(id).unwrap(battery_ptr); err) {
			return pxe::error("failed to get battery display component", *err);
		}

		if(!battery_ptr->is_visible()) {
			continue;
		}

		const auto display_idx = battery_ptr->get_index();
		const auto puzzle_idx = battery_order.at(display_idx);

		if(auto &bat = current_puzzle_.at(puzzle_idx); bat.closed()) {
			continue;
		}

		const auto battery_pos = battery_ptr->get_position();
		const auto delta_x = battery_pos.x - focus_pos.x;
		const auto delta_y = battery_pos.y - focus_pos.y;

		const auto is_correct_direction =
			(dx != 0 && std::signbit(delta_x) == std::signbit(static_cast<float>(dx)) && std::abs(delta_x) > 1.0F)
			|| (dy != 0 && std::signbit(delta_y) == std::signbit(static_cast<float>(dy)) && std::abs(delta_y) > 1.0F);

		if(!is_correct_direction) {
			continue;
		}

		if(const auto distance = std::sqrt((delta_x * delta_x) + (delta_y * delta_y)); distance < closest_distance) {
			closest_distance = distance;
			closest_id = id;
		}
	}

	if(closest_id.has_value()) {
		std::shared_ptr<battery_display> new_focus_ptr;
		if(const auto err = get_component<battery_display>(*closest_id).unwrap(new_focus_ptr); err) {
			return pxe::error("failed to get new focussed battery display", *err);
		}
		focus_battery_ptr->set_focussed(false);
		new_focus_ptr->set_focussed(true);
	}

	return true;
}

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
			SPDLOG_WARN("no free spark found to play animation");
		}
	}
	return true;
}

} // namespace energy