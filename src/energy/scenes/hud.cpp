// SPDX-FileCopyrightText: 2026 Juan Medina
// SPDX-License-Identifier: MIT

#include "hud.hpp"

#include "../../engine/app.hpp"
#include "../../engine/components/button.hpp"
#include "../../engine/components/component.hpp"
#include "../../engine/components/quick_bar.hpp"
#include "../../engine/result.hpp"
#include "../../engine/scenes/scene.hpp"

#include <format>
#include <memory>
#include <optional>

namespace energy {

auto hud::init(engine::app &app) -> engine::result<> {
	if(const auto err = scene::init(app).unwrap(); err) {
		return engine::error("failed to initialize base HUD scene", *err);
	}

	if(const auto err = register_component<engine::quick_bar>(sprite_sheet, normal, hover, gap).unwrap(quick_bar_);
	   err) {
		return engine::error("failed to initialize quick bar", *err);
	}

	std::shared_ptr<engine::quick_bar> quick_bar;
	if(const auto err = get_component<engine::quick_bar>(quick_bar_).unwrap(quick_bar); err) {
		return engine::error("failed to get HUD quick bar", *err);
	}

	if(const auto err = quick_bar->add_button(fullscreen_frame).unwrap(toggle_fullscreen_button_); err) {
		return engine::error("failed to add quick bar slot button", *err);
	}

#ifndef __EMSCRIPTEN__
	if(const auto err = quick_bar->add_button("cross.png").unwrap(close_button_); err) {
		return engine::error("failed to add quick bar slot button", *err);
	}
#endif

	button_click_ = app.bind_event<engine::button::click>(this, &hud::on_button_click);

	return true;
}

auto hud::layout(const engine::size size) -> engine::result<> {
	std::shared_ptr<engine::quick_bar> quick_bar;
	if(const auto err = get_component<engine::quick_bar>(quick_bar_).unwrap(quick_bar); err) {
		return engine::error("failed to get HUD quick bar for layout", *err);
	}

	auto constexpr bar_gap = 5.0F;
	auto [width, height] = quick_bar->get_size();

	const auto x = size.width - (width / 2) - bar_gap;
	const auto y = bar_gap + (height / 2);
	quick_bar->set_position({
		.x = x,
		.y = y,
	});

	return scene::layout(size);
}

auto hud::end() -> engine::result<> {
	get_app().unsubscribe(button_click_);
	return scene::end();
}

auto hud::on_button_click(const engine::button::click &evt) -> engine::result<> {
	if(evt.id == close_button_) {
		engine::app::close();
	} else if(evt.id == toggle_fullscreen_button_) {
		auto const full_screen = get_app().toggle_fullscreen();
		std::shared_ptr<engine::quick_bar> quick_bar;
		if(const auto err = get_component<engine::quick_bar>(quick_bar_).unwrap(quick_bar); err) {
			return engine::error("failed to get HUD quick bar for layout", *err);
		}
		if(const auto err =
			   quick_bar
				   ->set_button_frame_name(toggle_fullscreen_button_, full_screen ? windowed_frame : fullscreen_frame)
				   .unwrap();
		   err) {
			return engine::error("failed to set toggle fullscreen button frame", *err);
		}
	}
	return true;
}

} // namespace energy