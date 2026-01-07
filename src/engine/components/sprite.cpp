// SPDX-FileCopyrightText: 2026 Juan Medina
// SPDX-License-Identifier: MIT

#include "sprite.hpp"

#include "../app.hpp"

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

	if(const auto err = get_app().draw_sprite(sprite_sheet_, frame_, get_pos(), tint_).ko(); err) {
		return error("failed to draw sprite", *err);
	}
	return component::draw();
}

} // namespace engine