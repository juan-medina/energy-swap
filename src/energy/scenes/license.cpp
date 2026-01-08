// SPDX-FileCopyrightText: 2026 Juan Medina
// SPDX-License-Identifier: MIT

#include "license.hpp"

#include "../../engine/app.hpp"

#include <spdlog/spdlog.h>

namespace energy {

auto license::init(engine::app &app) -> engine::result<> {
	if(const auto err = scene::init(app).ko(); err) {
		return engine::error("failed to initialize base component", *err);
	}

	SPDLOG_INFO("license scene initialized");

	if(const auto err = scroll_text_.init(app).ko(); err) {
		return engine::error("failed to initialize scroll text component", *err);
	}

	char *text = nullptr;
	if(text = LoadFileText(license_path); text == nullptr) {
		return engine::error(std::format("failed to load license file from {}", license_path));
	}

	scroll_text_.set_text(text);
	UnloadFileText(text);
	scroll_text_.set_position({.x = 10, .y = 10});
	scroll_text_.set_size({.width = 500, .height = 400});
	scroll_text_.set_title("License");

	if(const auto err = accept_button_.init(app).ko(); err) {
		return engine::error("failed to initialize accept button component", *err);
	}

	accept_button_.set_text("Accept");
	accept_button_.set_position({.x = 0, .y = 0});
	accept_button_.set_size({.width = 60, .height = 30});

	button_click_ = app.bind_event<engine::button::click>(this, &license::on_button_click);

	return true;
}

auto license::end() -> engine::result<> {
	get_app().unsubscribe(button_click_);
	return scene::end();
}

auto license::update(float delta) -> engine::result<> {
	if(const auto err = scroll_text_.update(delta).ko(); err) {
		return engine::error("failed to update scroll text component", *err);
	}

	if(const auto err = accept_button_.update(delta).ko(); err) {
		return engine::error("failed to update accept button", *err);
	}
	return true;
}

auto license::draw() -> engine::result<> {
	if(const auto err = scroll_text_.draw().ko(); err) {
		return engine::error("failed to draw scroll text component", *err);
	}

	if(const auto err = accept_button_.draw().ko(); err) {
		return engine::error("failed to draw accept button", *err);
	}
	return true;
}

auto license::layout(const engine::size screen_size) -> void {
	const auto min_width = screen_size.width * 2.5F / 3.0F;
	scroll_text_.set_size({.width = std::min(min_width, 1200.0F), .height = screen_size.height * 3.5F / 5.0F});

	scroll_text_.set_position({.x = (screen_size.width - scroll_text_.get_size().width) / 2.0F,
							   .y = (screen_size.height - scroll_text_.get_size().height) / 2.0F});

	const auto [width, height] = accept_button_.get_size();
	float const button_x = (screen_size.width - width) / 2.0F;
	float const button_y = scroll_text_.get_position().y + scroll_text_.get_size().height + 10;
	accept_button_.set_position({.x = button_x, .y = button_y});
}

auto license::on_button_click(const engine::button::click &evt) -> engine::result<> {
	if(evt.id == accept_button_.get_id()) {
		get_app().post_event(accepted{});
	}
	return true;
}

} // namespace energy