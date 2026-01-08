// SPDX-FileCopyrightText: 2026 Juan Medina
// SPDX-License-Identifier: MIT

#include "game.hpp"

#include "../../engine/app.hpp"
#include "../energy_swap.hpp"

#include <array>
#include <ranges>
#include <spdlog/spdlog.h>

namespace energy {

auto game::init(engine::app &app) -> engine::result<> {
	if(const auto err = scene::init(app).ko(); err) {
		return engine::error("failed to initialize base component", *err);
	}

	SPDLOG_INFO("game scene initialized");

	if(const auto err = title_.init(app).ko(); err) {
		return engine::error("failed to initialize title label", *err);
	}

	if(const auto err = back_button_.init(app).ko(); err) {
		return engine::error("failed to initialize back button", *err);
	}

	if(const auto err = next_button_.init(app).ko(); err) {
		return engine::error("failed to initialize next button", *err);
	}

	if(const auto err = app.load_sprite_sheet(sprite_sheet_name, sprite_sheet_path).ko(); err) {
		return engine::error("failed to initialize sprite sheet", *err);
	}

	for(auto &battery_display: battery_displays_) {
		if(const auto err = battery_display.init(app).ko(); err) {
			return engine::error("failed to initialize battery sprite", *err);
		}
		battery_display.set_visible(false);
	}

	for(const auto index: std::views::iota(0, max_batteries)) {
		const auto ordered = battery_order.at(index);
		battery_displays_.at(index).set_battery(batteries_.at(ordered));
		battery_displays_.at(index).set_id(index);
	}

	battery_click_ = app.bind_event<battery_display::click>(this, &game::on_battery_click);

	back_button_.set_text("Back");
	back_button_.set_position({.x = 0, .y = 0});
	back_button_.set_size({.width = 45, .height = 25});

	next_button_.set_text("Next");
	next_button_.set_position({.x = 0, .y = 0});
	next_button_.set_size({.width = 45, .height = 25});

	button_click_ = app.bind_event<engine::button::click>(this, &game::on_button_click);

	return true;
}

auto game::end() -> engine::result<> {
	if(const auto err = title_.end().ko(); err) {
		return engine::error("failed to end logo texture", *err);
	}

	if(const auto err = back_button_.end().ko(); err) {
		return engine::error("failed to end back button", *err);
	}

	if(const auto err = next_button_.end().ko(); err) {
		return engine::error("failed to end next button", *err);
	}

	if(const auto err = get_app().unload_sprite_sheet(sprite_sheet_name).ko(); err) {
		return engine::error("failed to end sprite sheet", *err);
	}

	for(auto &sprite: battery_displays_) {
		if(const auto err = sprite.end().ko(); err) {
			return engine::error("failed to end battery display", *err);
		}
	}

	get_app().unsubscribe(button_click_);

	return scene::end();
}

auto game::update(const float delta) -> engine::result<> {
	for(auto &sprite: battery_displays_) {
		if(const auto err = sprite.update(delta).ko(); err) {
			return engine::error("failed to update battery display", *err);
		}
	}
	return true;
}

auto game::draw() -> engine::result<> {
	if(const auto err = title_.draw().ko(); err) {
		return engine::error("failed to draw title label", *err);
	}

	if(const auto err = back_button_.draw().ko(); err) {
		return engine::error("failed to draw back button", *err);
	}

	if(const auto err = next_button_.draw().ko(); err) {
		return engine::error("failed to draw next button", *err);
	}

	for(auto &sprite: battery_displays_) {
		if(const auto err = sprite.draw().ko(); err) {
			return engine::error("failed to draw battery sprite", *err);
		}
	}

	return true;
}

auto game::layout(const engine::size screen_size) -> void {
	const auto [label_width, label_height] = title_.get_size();
	title_.set_position({
		.x = (screen_size.width - label_width) / 2.0F,
		.y = 10.0F,
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
}

auto game::toggle_batteries(const size_t number) -> void {
	auto index = static_cast<size_t>(0);
	for(const auto battery_num: battery_order) {
		battery_displays_.at(index).set_visible(battery_num < number);
		++index;
	}
}

auto game::setup_puzzle(const std::string &puzzle_str) -> engine::result<> {
	auto [puzzle, error] = puzzle::from_string(puzzle_str).ok();
	if(error) {
		return engine::error("failed to parse puzzle from string: {}", *error);
	}
	current_puzzle_ = *puzzle;
	auto const total_batteries = current_puzzle_.size();
	toggle_batteries(total_batteries);

	for(auto i = static_cast<size_t>(0); i < total_batteries; ++i) {
		batteries_.at(i) = current_puzzle_.at(i);
	}

	return true;
}

auto game::enable() -> engine::result<> {
	const auto &app = dynamic_cast<energy_swap &>(get_app());

	title_.set_text(std::format("Level {}", app.get_current_level()));
	title_.set_font_size(30);

	if(const auto err = scene::enable().ko(); err) {
		return engine::error("failed to enable base scene", *err);
	}

	if(const auto err = get_app().play_music(game_music, 0.5F).ko(); err) {
		return engine::error("fail to play game music", *err);
	}

	const auto &level_str = app.get_current_level_string();

	SPDLOG_DEBUG("setting up puzzle with level string: {}", level_str);

	if(const auto err = setup_puzzle(level_str).ko(); err) {
		return engine::error("failed to setup puzzle", *err);
	}

	return true;
}

auto game::on_battery_click(const battery_display::click &click) -> engine::result<> {
	const auto clicked_index = battery_order.at(click.index);

	// find if we have a selected battery
	//	clang-tidy, only in WSL2, complains that we should use auto *const, when this is not a pointer
	const auto selected_it = // NOLINT(*-qualified-auto)
		std::ranges::find_if(battery_displays_, [](const battery_display &battery_display) -> bool {
			return battery_display.is_selected();
		});

	// if we have none
	if(selected_it == battery_displays_.end()) {
		// select the clicked battery if it's not closed or empty
		if(!batteries_.at(clicked_index).closed() && !batteries_.at(clicked_index).empty()) {
			battery_displays_.at(click.index).set_selected(true);
		}
		return true;
	}

	// deselect it
	selected_it->set_selected(false);

	const auto selected_index = battery_order.at(selected_it->get_id());
	auto &from_battery = batteries_.at(selected_index);

	if(auto &to_battery = batteries_.at(clicked_index); to_battery.can_get_from(from_battery)) {
		to_battery.transfer_energy_from(from_battery);
	}

	return true;
}

auto game::on_button_click(const engine::button::click &evt) -> engine::result<> {
	if(evt.id == next_button_.get_id()) {
		get_app().post_event(next_level{});
	} else if(evt.id == back_button_.get_id()) {
		get_app().post_event(back{});
	}

	return true;
}

} // namespace energy