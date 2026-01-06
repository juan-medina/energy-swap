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

	title_.set_text("Game");
	title_.set_font_size(60);

	return true;
}

auto game::end() -> engine::result<> {

	if(const auto err = title_.end().ko(); err) {
		return engine::error("failed to end logo texture", *err);
	}

	return scene::end();
}

auto game::update(const float /*delta*/) -> engine::result<> {
	return true;
}

auto game::draw() -> engine::result<> {
	if(const auto err = title_.draw().ko(); err) {
		return engine::error("failed to draw logo texture", *err);
	}

	return true;
}

auto game::layout(const engine::size screen_size) -> void {
	const auto [label_width, label_height] = title_.get_size();
	title_.set_position({
		.x = (screen_size.width - label_width) / 2.0F,
		.y = (screen_size.height * 0.2F) - (label_height / 2.0F),
	});
}

} // namespace energy