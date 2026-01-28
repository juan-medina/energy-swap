// SPDX-FileCopyrightText: 2026 Juan Medina
// SPDX-License-Identifier: MIT

#include "mode.hpp"

#include <pxe/app.hpp>
#include <pxe/components/button.hpp>
#include <pxe/components/component.hpp>
#include <pxe/components/label.hpp>
#include <pxe/result.hpp>
#include <pxe/scenes/scene.hpp>

#include "../level_manager.hpp"

#include <raylib.h>

#include <memory>
#include <raygui.h>
#include <string>

namespace energy {

// =============================================================================
// Initialization & Shutdown
// =============================================================================

auto mode::init(pxe::app &app) -> pxe::result<> {
	if(const auto err = scene::init(app).unwrap(); err) {
		return pxe::error("failed to initialize base scene", *err);
	}
	if(const auto err = register_component<pxe::label>().unwrap(title_label_); err) {
		return pxe::error("failed to register title label", *err);
	}
	if(const auto err = register_component<pxe::button>().unwrap(classic_button_); err) {
		return pxe::error("failed to register classic button", *err);
	}
	if(const auto err = register_component<pxe::button>().unwrap(cosmic_button_); err) {
		return pxe::error("failed to register cosmic button", *err);
	}
	if(const auto err = register_component<pxe::button>().unwrap(back_button_); err) {
		return pxe::error("failed to register back button", *err);
	}

	std::shared_ptr<pxe::label> title;
	if(const auto err = get_component<pxe::label>(title_label_).unwrap(title); err) {
		return pxe::error("failed to get title label", *err);
	}
	title->set_text("Select Game Mode");
	title->set_font_size(32);
	title->set_centered(true);

	std::shared_ptr<pxe::button> classic_btn;
	if(const auto err = get_component<pxe::button>(classic_button_).unwrap(classic_btn); err) {
		return pxe::error("failed to get classic button", *err);
	}
	classic_btn->set_text(GuiIconText(ICON_PLAYER, "Classic"));
	classic_btn->set_size({.width = 120, .height = 40});
	classic_btn->set_font_size(30);
	classic_btn->set_controller_button(GAMEPAD_BUTTON_RIGHT_FACE_DOWN);

	std::shared_ptr<pxe::button> cosmic_btn;
	if(const auto err = get_component<pxe::button>(cosmic_button_).unwrap(cosmic_btn); err) {
		return pxe::error("failed to get cosmic button", *err);
	}
	cosmic_btn->set_text(GuiIconText(ICON_STAR, "Cosmic"));
	cosmic_btn->set_size({.width = 120, .height = 40});
	cosmic_btn->set_font_size(30);
	cosmic_btn->set_controller_button(GAMEPAD_BUTTON_RIGHT_FACE_UP);

	std::shared_ptr<pxe::button> back_btn;
	if(const auto err = get_component<pxe::button>(back_button_).unwrap(back_btn); err) {
		return pxe::error("failed to get back button", *err);
	}
	back_btn->set_text(GuiIconText(ICON_PLAYER_PREVIOUS, "Back"));
	back_btn->set_size({.width = 70, .height = 25});
	back_btn->set_font_size(button_font_size);
	back_btn->set_controller_button(GAMEPAD_BUTTON_RIGHT_FACE_RIGHT);

	button_click_ = app.bind_event<pxe::button::click>(this, &mode::on_button_click);
	return true;
}

auto mode::end() -> pxe::result<> {
	get_app().unsubscribe(button_click_);
	return scene::end();
}

auto mode::update(float delta) -> pxe::result<> {
	return scene::update(delta);
}

auto mode::show() -> pxe::result<> {
	return scene::show();
}

auto mode::layout(const pxe::size screen_size) -> pxe::result<> {
	std::shared_ptr<pxe::button> classic_btn;
	if(const auto err = get_component<pxe::button>(classic_button_).unwrap(classic_btn); err) {
		return pxe::error("failed to get classic button", *err);
	}
	std::shared_ptr<pxe::button> cosmic_btn;
	if(const auto err = get_component<pxe::button>(cosmic_button_).unwrap(cosmic_btn); err) {
		return pxe::error("failed to get cosmic button", *err);
	}
	std::shared_ptr<pxe::button> back_btn;
	if(const auto err = get_component<pxe::button>(back_button_).unwrap(back_btn); err) {
		return pxe::error("failed to get back button", *err);
	}

	std::shared_ptr<pxe::label> title;
	if(const auto err = get_component<pxe::label>(title_label_).unwrap(title); err) {
		return pxe::error("failed to get title label", *err);
	}

	const auto [clw, clh] = classic_btn->get_size();
	const auto [cow, coh] = cosmic_btn->get_size();
	const auto [bca, bch] = back_btn->get_size();

	constexpr float gap = 40.0F;
	constexpr auto button_v_gap = 20.0F;
	const float total_width = clw + gap + cow;
	const float start_x = (screen_size.width - total_width) / 2.0F;
	const float y = screen_size.height / 2.0F;

	title->set_position({
		.x = screen_size.width / 2.0F,
		.y = button_v_gap,
	});

	classic_btn->set_position({.x = start_x, .y = y});
	cosmic_btn->set_position({.x = start_x + clw + gap, .y = y});

	back_btn->set_position({
		.x = (screen_size.width - bca) / 2.0F,
		.y = screen_size.height - bch - button_v_gap,
	});

	return true;
}

auto mode::on_button_click(const pxe::button::click &evt) -> pxe::result<> {
	if(evt.id == classic_button_) {
		get_app().post_event(selected{.mode = level_manager::mode::classic});
	} else if(evt.id == cosmic_button_) {
		get_app().post_event(selected{.mode = level_manager::mode::cosmic});
	} else if(evt.id == back_button_) {
		get_app().post_event(back{});
	}
	return true;
}

} // namespace energy
