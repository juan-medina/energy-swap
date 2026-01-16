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

	if(const auto err = register_component<pxe::label>().unwrap(title_); err) {
		return pxe::error("failed to register title label", *err);
	}

	if(const auto err = register_component<pxe::label>().unwrap(status_); err) {
		return pxe::error("failed to register status label", *err);
	}

	if(const auto err = register_component<pxe::button>().unwrap(back_button_); err) {
		return pxe::error("failed to register back button", *err);
	}

	if(const auto err = register_component<pxe::button>().unwrap(next_button_); err) {
		return pxe::error("failed to register next button", *err);
	}

	if(const auto err = register_component<pxe::button>().unwrap(reset_button_); err) {
		return pxe::error("failed to register reset button", *err);
	}

	if(const auto err = app.load_sprite_sheet(sprite_sheet_name, sprite_sheet_path).unwrap(); err) {
		return pxe::error("failed to initialize sprite sheet", *err);
	}

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
		const auto ordered = battery_order.at(index);

		const auto id = battery_displays_.at(index);
		std::shared_ptr<battery_display> battery_display_ptr;
		if(const auto err = get_component<battery_display>(id).unwrap(battery_display_ptr); err) {
			return pxe::error("failed to get battery display component", *err);
		}
		battery_display_ptr->set_battery(batteries_.at(ordered));
		battery_display_ptr->set_index(index);
	}

	battery_click_ = app.bind_event<battery_display::click>(this, &game::on_battery_click);

	std::shared_ptr<pxe::button> back_button_ptr;
	if(const auto err = get_component<pxe::button>(back_button_).unwrap(back_button_ptr); err) {
		return pxe::error("failed to get back button", *err);
	}

	std::shared_ptr<pxe::button> next_button_ptr;
	if(const auto err = get_component<pxe::button>(next_button_).unwrap(next_button_ptr); err) {
		return pxe::error("failed to get back button", *err);
	}

	std::shared_ptr<pxe::button> reset_button_ptr;
	if(const auto err = get_component<pxe::button>(reset_button_).unwrap(reset_button_ptr); err) {
		return pxe::error("failed to get back button", *err);
	}

	back_button_ptr->set_text("Back");
	back_button_ptr->set_position({.x = 0, .y = 0});
	back_button_ptr->set_size({.width = 45, .height = 25});

	next_button_ptr->set_text("Next");
	next_button_ptr->set_position({.x = 0, .y = 0});
	next_button_ptr->set_size({.width = 45, .height = 25});

	reset_button_ptr->set_text("Reset");
	reset_button_ptr->set_position({.x = 0, .y = 0});
	reset_button_ptr->set_size({.width = 45, .height = 25});

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
	if(const auto err = get_app().unload_sprite_sheet(sprite_sheet_name).unwrap(); err) {
		return pxe::error("failed to end sprite sheet", *err);
	}

	get_app().unsubscribe(button_click_);

	return scene::end();
}

auto game::layout(const pxe::size screen_size) -> pxe::result<> {
	std::shared_ptr<pxe::label> title_ptr;
	if(const auto err = get_component<pxe::label>(title_).unwrap(title_ptr); err) {
		return pxe::error("failed to get title label", *err);
	}

	title_ptr->set_position({
		.x = screen_size.width / 2.0F,
		.y = 10.0F,
	});

	std::shared_ptr<pxe::label> status_ptr;
	if(const auto err = get_component<pxe::label>(status_).unwrap(status_ptr); err) {
		return pxe::error("failed to get status label", *err);
	}

	status_ptr->set_position({
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
		const auto id = battery_displays_.at(i);
		std::shared_ptr<battery_display> battery_ptr;
		if(const auto err = get_component<battery_display>(id).unwrap(battery_ptr); err) {
			return pxe::error("failed to get battery display component", *err);
		}
		battery_ptr->set_position({.x = pos_x, .y = pos_y});
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
		return pxe::error("failed to get next button", *err);
	}

	constexpr auto button_v_gap = 10.0F;
	constexpr auto button_h_gap = 10.0F;

	const auto [button_width, button_height] = back_button_ptr->get_size(); // assuming all buttons have the same size

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
		return pxe::error("failed to parse puzzle from string: {}", *error);
	}

	auto const total_batteries = current_puzzle_.size();
	toggle_batteries(total_batteries);

	for(const auto index: std::views::iota(0, max_batteries)) {
		const auto id = battery_displays_.at(index);
		std::shared_ptr<battery_display> battery_ptr;
		if(const auto err = get_component<battery_display>(id).unwrap(battery_ptr); err) {
			return pxe::error("failed to get battery display component", *err);
		}
		battery_ptr->reset();
	}

	for(auto i = static_cast<size_t>(0); i < total_batteries; ++i) {
		batteries_.at(i) = current_puzzle_.at(i);
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

	if(const auto err = scene::show().unwrap(); err) {
		return pxe::error("failed to enable base scene", *err);
	}

	if(const auto err = get_app().play_music(game_music).unwrap(); err) {
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
		const auto id = battery_displays_.at(index);
		std::shared_ptr<battery_display> battery_ptr;
		if(const auto err = get_component<battery_display>(id).unwrap(battery_ptr); err) {
			continue;
		}
		battery_ptr->set_visible(battery_num < number);
		++index;
	}
}

auto game::on_battery_click(const battery_display::click &click) -> pxe::result<> {
	const auto clicked_index = battery_order.at(click.index);

	const auto click_index_id = battery_displays_.at(click.index);
	std::shared_ptr<battery_display> click_index_battery_ptr;
	if(const auto err = get_component<battery_display>(click_index_id).unwrap(click_index_battery_ptr); err) {
		return pxe::error("failed to get battery display component", *err);
	}

	// find if we have a selected battery
	//	clang-tidy, only in WSL2, complains that we should use auto *const, when this is not a pointer
	const auto selected_it = // NOLINT(*-qualified-auto)
		std::ranges::find_if(battery_displays_, [this](const auto &id) -> bool {
			std::shared_ptr<battery_display> battery_ptr;
			if(const auto err = get_component<battery_display>(id).unwrap(battery_ptr); err) {
				return false;
			}
			return battery_ptr->is_selected();
		});

	// if we have none
	if(selected_it == battery_displays_.end()) {
		// play click sound only for selecting
		if(const auto err = get_app().play_sfx(battery_click_sound).unwrap(); err) {
			return pxe::error("failed to play battery click sound", *err);
		}

		// select the clicked battery if it's not closed or empty
		if(!batteries_.at(clicked_index).closed() && !batteries_.at(clicked_index).empty()) {
			click_index_battery_ptr->set_selected(true);
		}
		return true;
	}

	std::shared_ptr<battery_display> selected_battery_ptr;
	if(const auto err = get_component<battery_display>(*selected_it).unwrap(selected_battery_ptr); err) {
		return false;
	}

	// deselect it
	selected_battery_ptr->set_selected(false);

	auto need_click_sound = true;

	const auto selected_index = battery_order.at(selected_battery_ptr->get_index());
	auto &from_battery = batteries_.at(selected_index);

	if(auto &to_battery = batteries_.at(clicked_index);
	   (selected_index != clicked_index) && to_battery.can_get_from(from_battery)) {
		// sparks
		if(const auto err = shoot_sparks(selected_battery_ptr->get_position(),
										 click_index_battery_ptr->get_position(),
										 selected_battery_ptr->get_top_color(),
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
	auto game_ended = false;

	std::shared_ptr<pxe::label> status_ptr;
	if(const auto err = get_component<pxe::label>(status_).unwrap(status_ptr); err) {
		return pxe::error("failed to get status label", *err);
	}

	if(current_puzzle_.is_solved()) {
		status_ptr->set_text("You Win, continue to the next level ...");
		std::shared_ptr<pxe::button> next_button_ptr;
		if(const auto err = get_component<pxe::button>(next_button_).unwrap(next_button_ptr); err) {
			return pxe::error("failed to get next button", *err);
		}

		std::shared_ptr<pxe::button> reset_button_ptr;
		if(const auto err = get_component<pxe::button>(reset_button_).unwrap(reset_button_ptr); err) {
			return pxe::error("failed to get next button", *err);
		}

		next_button_ptr->set_visible(true);
		reset_button_ptr->set_visible(false);
		game_ended = true;
	} else if(!current_puzzle_.is_solvable()) {
		status_ptr->set_text("No more moves available, try again ...");
		game_ended = true;
	}

	if(game_ended) {
		for(const auto &id: battery_displays_) {
			std::shared_ptr<battery_display> battery_ptr;
			if(const auto err = get_component<battery_display>(id).unwrap(battery_ptr); err) {
				return pxe::error("failed to get battery display component", *err);
			}
			battery_ptr->set_enabled(false);
		}
	}
	return true;
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
	if(const auto err = get_app().play_sfx(zap_sound).unwrap(); err) {
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