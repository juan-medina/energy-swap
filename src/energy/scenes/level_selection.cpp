// SPDX-FileCopyrightText: 2026 Juan Medina
// SPDX-License-Identifier: MIT

#include "level_selection.hpp"

#include <pxe/app.hpp>
#include <pxe/components/button.hpp>
#include <pxe/components/label.hpp>
#include <pxe/result.hpp>
#include <pxe/scenes/scene.hpp>

#include "../energy_swap.hpp"

#include <array>
#include <cstddef>
#include <format>
#include <memory>
#include <spdlog/spdlog.h>

namespace energy {

auto level_selection::init(pxe::app &app) -> pxe::result<> {
	if(const auto err = scene::init(app).unwrap(); err) {
		return pxe::error("failed to initialize base scene", *err);
	}

	SPDLOG_INFO("level selection scene initialized");

	if(const auto err = register_component<pxe::label>().unwrap(title_); err) {
		return pxe::error("failed to register title label", *err);
	}

	for(auto &button_id: level_buttons_) {
		if(const auto err = register_component<pxe::button>().unwrap(button_id); err) {
			return pxe::error("failed to register level button", *err);
		}
	}

	if(const auto err = register_component<pxe::button>().unwrap(prev_page_button_); err) {
		return pxe::error("failed to register prev page button", *err);
	}

	if(const auto err = register_component<pxe::button>().unwrap(next_page_button_); err) {
		return pxe::error("failed to register next page button", *err);
	}

	std::shared_ptr<pxe::button> prev_button_ptr;
	if(const auto err = get_component<pxe::button>(prev_page_button_).unwrap(prev_button_ptr); err) {
		return pxe::error("failed to get prev page button", *err);
	}

	std::shared_ptr<pxe::button> next_button_ptr;
	if(const auto err = get_component<pxe::button>(next_page_button_).unwrap(next_button_ptr); err) {
		return pxe::error("failed to get next page button", *err);
	}

	prev_button_ptr->set_text("<");
	prev_button_ptr->set_size({.width = 45, .height = 25});
	prev_button_ptr->set_font_size(button_font_size);

	next_button_ptr->set_text(">");
	next_button_ptr->set_size({.width = 45, .height = 25});
	next_button_ptr->set_font_size(button_font_size);

	for(size_t i = 0; i < max_level_buttons; ++i) {
		std::shared_ptr<pxe::button> button_ptr;
		if(const auto err = get_component<pxe::button>(level_buttons_.at(i)).unwrap(button_ptr); err) {
			return pxe::error("failed to get level button", *err);
		}
		button_ptr->set_size({.width = 50, .height = 50});
		button_ptr->set_font_size(button_font_size);
	}

	button_click_ = app.bind_event<pxe::button::click>(this, &level_selection::on_button_click);

	return true;
}

auto level_selection::end() -> pxe::result<> {
	get_app().unsubscribe(button_click_);
	return scene::end();
}

auto level_selection::layout(const pxe::size screen_size) -> pxe::result<> {
	std::shared_ptr<pxe::label> title_ptr;
	if(const auto err = get_component<pxe::label>(title_).unwrap(title_ptr); err) {
		return pxe::error("failed to get title label", *err);
	}

	title_ptr->set_position({
		.x = screen_size.width / 2.0F,
		.y = 20.0F,
	});

	// Layout level buttons in a 2x5 grid
	constexpr int cols = 5;
	constexpr int rows = 2;
	constexpr auto button_spacing = 10.0F;
	constexpr auto grid_width = (50.0F * cols) + (button_spacing * (cols - 1));
	constexpr auto grid_height = (50.0F * rows) + (button_spacing * (rows - 1));
	const auto start_x = ((screen_size.width - grid_width) / 2.0F) + 25.0F;
	const auto start_y = ((screen_size.height - grid_height) / 2.0F) + 25.0F;

	for(size_t i = 0; i < max_level_buttons; ++i) {
		const auto row = i / cols;
		const auto col = i % cols;
		const auto pos_x = start_x + ((50.0F + button_spacing) * static_cast<float>(col));
		const auto pos_y = start_y + ((50.0F + button_spacing) * static_cast<float>(row));

		std::shared_ptr<pxe::button> button_ptr;
		if(const auto err = get_component<pxe::button>(level_buttons_.at(i)).unwrap(button_ptr); err) {
			return pxe::error("failed to get level button", *err);
		}
		button_ptr->set_position({.x = pos_x, .y = pos_y});
	}

	std::shared_ptr<pxe::button> prev_button_ptr;
	if(const auto err = get_component<pxe::button>(prev_page_button_).unwrap(prev_button_ptr); err) {
		return pxe::error("failed to get prev page button", *err);
	}

	std::shared_ptr<pxe::button> next_button_ptr;
	if(const auto err = get_component<pxe::button>(next_page_button_).unwrap(next_button_ptr); err) {
		return pxe::error("failed to get next page button", *err);
	}

	constexpr auto button_v_gap = 10.0F;
	const auto center_pos_x = screen_size.width * 0.5F;
	const auto [button_width, button_height] = prev_button_ptr->get_size();
	const auto center_pos_y = screen_size.height - button_height - button_v_gap;

	constexpr auto page_button_spacing = 150.0F;

	prev_button_ptr->set_position({
		.x = center_pos_x - page_button_spacing,
		.y = center_pos_y,
	});

	next_button_ptr->set_position({
		.x = center_pos_x + page_button_spacing,
		.y = center_pos_y,
	});

	return true;
}

auto level_selection::show() -> pxe::result<> {
	auto &app = dynamic_cast<energy_swap &>(get_app());

	// Load max reached level from settings
	max_reached_level_ = app.get_max_reached_level();

	// Get current level from app
	const auto current_level = app.get_current_level();

	// Calculate which page the current level is on
	current_page_ = (current_level - 1) / levels_per_page;

	std::shared_ptr<pxe::label> title_ptr;
	if(const auto err = get_component<pxe::label>(title_).unwrap(title_ptr); err) {
		return pxe::error("failed to get title label", *err);
	}

	title_ptr->set_text("Select Level");
	title_ptr->set_font_size(30);
	title_ptr->set_centered(true);

	if(const auto err = update_buttons().unwrap(); err) {
		return pxe::error("failed to update buttons", *err);
	}

	if(const auto err = scene::show().unwrap(); err) {
		return pxe::error("failed to show base scene", *err);
	}

	return true;
}

auto level_selection::update_buttons() -> pxe::result<> {
	auto &app = dynamic_cast<energy_swap &>(get_app());
	const auto current_level = app.get_current_level();
	const auto start_level = (current_page_ * levels_per_page) + 1;

	for(size_t i = 0; i < max_level_buttons; ++i) {
		const auto level = start_level + i;

		std::shared_ptr<pxe::button> button_ptr;
		if(const auto err = get_component<pxe::button>(level_buttons_.at(i)).unwrap(button_ptr); err) {
			return pxe::error("failed to get level button", *err);
		}

		if(level == current_level) {
			button_ptr->set_text(std::format("> {} <", level));
		} else {
			button_ptr->set_text(std::format("{}", level));
		}

		// Enable button only if level is unlocked
		button_ptr->set_enabled(level <= max_reached_level_);
	}

	std::shared_ptr<pxe::button> prev_button_ptr;
	if(const auto err = get_component<pxe::button>(prev_page_button_).unwrap(prev_button_ptr); err) {
		return pxe::error("failed to get prev page button", *err);
	}

	std::shared_ptr<pxe::button> next_button_ptr;
	if(const auto err = get_component<pxe::button>(next_page_button_).unwrap(next_button_ptr); err) {
		return pxe::error("failed to get next page button", *err);
	}

	prev_button_ptr->set_enabled(current_page_ > 0);
	next_button_ptr->set_enabled(current_page_ < total_pages - 1);

	return true;
}

auto level_selection::on_button_click(const pxe::button::click &evt) -> pxe::result<> {
	if(evt.id == prev_page_button_) {
		if(current_page_ > 0) {
			current_page_--;
			if(const auto err = update_buttons().unwrap(); err) {
				return pxe::error("failed to update buttons", *err);
			}
		}
	} else if(evt.id == next_page_button_) {
		if(current_page_ < total_pages - 1) {
			current_page_++;
			if(const auto err = update_buttons().unwrap(); err) {
				return pxe::error("failed to update buttons", *err);
			}
		}
	} else {
		// Check if it's a level button
		for(size_t i = 0; i < max_level_buttons; ++i) {
			if(evt.id == level_buttons_.at(i)) {
				const auto level = (current_page_ * levels_per_page) + i + 1;
				if(level <= max_reached_level_) {
					get_app().post_event(energy_swap::level_selected{.level = level});
				}
				break;
			}
		}
	}

	return true;
}

} // namespace energy