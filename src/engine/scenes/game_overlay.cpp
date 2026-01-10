// SPDX-FileCopyrightText: 2026 Juan Medina
// SPDX-License-Identifier: MIT

#include "game_overlay.hpp"

#include "../app.hpp"
#include "../components/component.hpp"
#include "../components/version_display.hpp"
#include "../result.hpp"
#include "scene.hpp"

#include <format>
#include <functional>
#include <memory>
#include <optional>

namespace engine {

auto game_overlay::init(app &app) -> result<> {
	if(const auto err = scene::init(app).unwrap(); err) {
		return error("failed to initialize base component", *err);
	}

	if(const auto err = register_component<version_display>().unwrap(version_display_); err) {
		return error("failed to register version display component", *err);
	}

	using click = version_display::click;
	click_ = get_app().subscribe<click>([this](const click &) -> result<> {
		get_app().post_event(version_click{});
		return true;
	});

	return true;
}

auto game_overlay::end() -> result<> {
	get_app().unsubscribe(click_);
	return scene::end();
}

auto game_overlay::layout(const size screen_size) -> result<> {
	std::shared_ptr<version_display> version;
	if(const auto err = get_component<version_display>(version_display_).unwrap(version); err) {
		return error("failed to get version display component", err);
	}

	// position version display at bottom-right corner with margin
	const auto [width, height] = version->get_size();
	version->set_position({
		.x = screen_size.width - width - margin,
		.y = screen_size.height - height - margin,
	});

	return true;
}

} // namespace engine