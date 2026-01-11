// SPDX-FileCopyrightText: 2026 Juan Medina
// SPDX-License-Identifier: MIT

#include "hud.hpp"

#include "../../engine/app.hpp"
#include "../../engine/components/component.hpp"
#include "../../engine/components/quick_bar.hpp"
#include "../../engine/result.hpp"
#include "../../engine/scenes/scene.hpp"

#include <cstddef>
#include <memory>

namespace energy {

auto hud::init(engine::app &app) -> engine::result<> {
	if(const auto err = scene::init(app).unwrap(); err) {
		return engine::error("failed to initialize base HUD scene", *err);
	}

	if(const auto err = register_component<engine::quick_bar>("sprites").unwrap(quick_bar_); err) {
		return engine::error("failed to initialize quick bar", *err);
	}

	std::shared_ptr<engine::quick_bar> quick_bar;
	if(const auto err = get_component<engine::quick_bar>(quick_bar_).unwrap(quick_bar); err) {
		return engine::error("failed to get HUD quick bar", *err);
	}

	size_t id = 0;
	if(const auto err = quick_bar->add_sprite("cross.png").unwrap(id); err) {
		return engine::error("failed to add quick bar slot sprite", *err);
	}
	if(const auto err = quick_bar->add_sprite("cross.png").unwrap(id); err) {
		return engine::error("failed to add quick bar slot sprite", *err);
	}
	if(const auto err = quick_bar->add_sprite("cross.png").unwrap(id); err) {
		return engine::error("failed to add quick bar slot sprite", *err);
	}

	return true;
}

auto hud::layout(const engine::size size) -> engine::result<> {
	std::shared_ptr<engine::quick_bar> quick_bar;
	if(const auto err = get_component<engine::quick_bar>(quick_bar_).unwrap(quick_bar); err) {
		return engine::error("failed to get HUD quick bar for layout", *err);
	}

	auto constexpr gap = 0.0F;
	auto [width, height] = quick_bar->get_size();

	const auto x = size.width - (width / 2) - gap;
	const auto y = gap + (height / 2);
	quick_bar->set_position({
		.x = x,
		.y = y,
	});

	return scene::layout(size);
}

} // namespace energy