// SPDX-FileCopyrightText: 2026 Juan Medina
// SPDX-LicenseCopyrightText: MIT

#include "quick_bar.hpp"

#include "../app.hpp"
#include "../result.hpp"
#include "component.hpp"
#include "sprite.hpp"

#include <raylib.h>

#include <algorithm>
#include <cstddef>
#include <memory>
#include <string>

namespace engine {

auto quick_bar::init(app &app, const std::string &sprite_sheet, float gap) -> result<> {
	if(const auto err = component::init(app).unwrap(); err) {
		return error("failed to initialize base component", *err);
	}

	sprite_sheet_ = sprite_sheet;
	gap_ = gap;

	return true;
}

auto quick_bar::add_sprite(const std::string &frame_name) -> result<size_t> {
	auto sprite_ptr = std::make_shared<sprite>();
	if(const auto err = sprite_ptr->init(get_app(), sprite_sheet_, frame_name).unwrap(); err) {
		return error("failed to initialize sprite in quick_bar", *err);
	}
	sprite_ptr->set_tint(Color(255, 255, 255, 60));
	sprites_.emplace_back(sprite_ptr);
	recalculate();

	return sprite_ptr->get_id();
}

auto quick_bar::set_position(const Vector2 &pos) -> void {
	component::set_position(pos);
	recalculate();
}

auto quick_bar::end() -> result<> {
	for(auto &sprite_ptr: sprites_) {
		if(const auto err = sprite_ptr->end().unwrap(); err) {
			return error("failed to end sprite in quick_bar", *err);
		}
	}
	sprites_.clear();

	return component::end();
}

auto quick_bar::update(float delta) -> result<> {
	for(const auto &sprite_ptr: sprites_) {
		if(const auto err = sprite_ptr->update(delta).unwrap(); err) {
			return error("failed to update sprite in quick_bar", *err);
		}
	}

	return component::update(delta);
}

auto quick_bar::draw() -> result<> {
	for(const auto &sprite_ptr: sprites_) {
		if(const auto err = sprite_ptr->draw().unwrap(); err) {
			return error("failed to draw sprite in quick_bar", *err);
		}
	}

	return component::draw();
}

auto quick_bar::recalculate() -> void {
	recalculate_size();
	const auto [width, height] = get_size();
	const auto [x, y] = get_position();

	float pos_x = x - (width / 2);
	const auto pos_y = y;

	for(const auto &sprite_ptr: sprites_) {
		const auto [sprite_width, _] = sprite_ptr->get_size();
		// we draw the sprite centered
		pos_x += sprite_width / 2;
		sprite_ptr->set_position({.x = pos_x, .y = pos_y});
		pos_x += sprite_width / 2;
		pos_x += gap_;
	}
}

auto quick_bar::recalculate_size() -> void {
	float total_width = 0.0F;
	float max_height = 0.0F;

	for(const auto &sprite_ptr: sprites_) {
		const auto [width, height] = sprite_ptr->get_size();
		total_width += width + gap_;
		max_height = std::max(max_height, height);
	}

	if(!sprites_.empty() && total_width > 0.0F) {
		total_width -= gap_;
	}

	set_size({.width = total_width, .height = max_height});
}

} // namespace engine
