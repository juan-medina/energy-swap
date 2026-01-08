// SPDX-FileCopyrightText: 2026 Juan Medina
// SPDX-License-Identifier: MIT

#include "sprite.hpp"

#include "../app.hpp"
#include "../result.hpp"
#include "component.hpp"
#include "raylib.h"

#include <optional>

namespace engine {

auto sprite::init(app &app) -> result<> {
	if(const auto err = component::init(app).ko(); err) {
		return error("failed to initialize base component", *err);
	}
	return error("sprite component requires sprite sheet and frame to be initialize");
}

auto sprite::init(app &app, const std::string &sprite_sheet, const std::string &frame) -> result<> {
	if(const auto err = component::init(app).ko(); err) {
		return error("failed to initialize base UI component", *err);
	}

	sprite_sheet_ = sprite_sheet;
	frame_ = frame;

	auto [size, err] = app.get_sprite_size(sprite_sheet_, frame_).ok();
	if(err) {
		return error("failed to get sprite size", *err);
	}

	original_size_ = *size;
	set_size(original_size_);

	const auto result = app.get_sprite_pivot(sprite_sheet_, frame_);
	if(result.has_error()) {
		return error("failed to get sprite pivot", result.get_error());
	}
	pivot_ = result.get_value();

	return true;
}

auto sprite::end() -> result<> {
	return component::end();
}
auto sprite::update(const float delta) -> result<> {
	return component::update(delta);
}

auto sprite::draw() -> result<> {
	if(!is_visible()) {
		return true;
	}

	if(const auto err = get_app().draw_sprite(sprite_sheet_, frame_, get_position(), scale_, tint_).ko(); err) {
		return error("failed to draw sprite", *err);
	}
	return component::draw();
}

auto sprite::set_scale(const float scale) -> void {
	scale_ = scale;
	set_size({.width = original_size_.width * scale_, .height = original_size_.height * scale_});
}
auto sprite::point_inside(const Vector2 point) const -> bool {
	const auto [pos_x, pos_y] = get_position();
	const auto size = get_size();
	const auto [pivot_x, pivot_y] = pivot_;

	return component::point_inside(
		{.x = pos_x - (pivot_x * size.width), .y = pos_y - (pivot_y * size.height)}, size, point);
}

} // namespace engine