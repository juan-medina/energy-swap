// SPDX-FileCopyrightText: 2026 Juan Medina
// SPDX-License-Identifier: MIT

#include "game.hpp"

#include "../../engine/app.hpp"

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

	title_.set_text("Level 1");
	title_.set_font_size(30);

	if(const auto err = app.load_sprite_sheet(sprite_sheet_name, sprite_sheet_path).ko(); err) {
		return engine::error("failed to initialize sprite sheet", *err);
	}

	for(auto &sprite: battery_sprites_) {
		if(const auto err = sprite.init(app, sprite_sheet_name, sprite_frame).ko(); err) {
			return engine::error("failed to initialize battery sprite", *err);
		}
		sprite.set_visible(false);
	}

	// show 6 batteries at start
	toggle_batteries(6);

	return true;
}

auto game::end() -> engine::result<> {
	if(const auto err = title_.end().ko(); err) {
		return engine::error("failed to end logo texture", *err);
	}

	if(const auto err = get_app().unload_sprite_sheet(sprite_sheet_name).ko(); err) {
		return engine::error("failed to end sprite sheet", *err);
	}

	for(auto &sprite: battery_sprites_) {
		if(const auto err = sprite.end().ko(); err) {
			return engine::error("failed to end battery sprite", *err);
		}
	}

	return scene::end();
}

auto game::update(const float /*delta*/) -> engine::result<> {
	return true;
}

auto game::draw() -> engine::result<> {
	if(const auto err = title_.draw().ko(); err) {
		return engine::error("failed to draw title label", *err);
	}

	for(auto &sprite: battery_sprites_) {
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
		battery_sprites_.at(i).set_position({.x = pos_x, .y = pos_y});
	}
}

#include <array>

auto game::toggle_batteries(const int number) -> void {
	int index = 0;
	for(const auto battery_num: battery_order) {
		battery_sprites_.at(index).set_visible(battery_num < number);
		++index;
	}
}

} // namespace energy