// SPDX-FileCopyrightText: 2026 Juan Medina
// SPDX-License-Identifier: MIT

#include "cosmic.hpp"

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

auto cosmic::init(pxe::app &app) -> pxe::result<> {
	if(const auto err = scene::init(app).unwrap(); err) {
		return pxe::error("failed to initialize base scene", *err);
	}
	if(const auto err = register_component<pxe::label>().unwrap(title_label_); err) {
		return pxe::error("failed to register title label", *err);
	}
	if(const auto err = register_component<pxe::button>().unwrap(normal_button_); err) {
		return pxe::error("failed to register normal button", *err);
	}
	if(const auto err = register_component<pxe::button>().unwrap(hard_button_); err) {
		return pxe::error("failed to register hard button", *err);
	}
	if(const auto err = register_component<pxe::button>().unwrap(burger_daddy_button_); err) {
		return pxe::error("failed to register burger daddy button", *err);
	}
	if(const auto err = register_component<pxe::button>().unwrap(back_button_); err) {
		return pxe::error("failed to register back button", *err);
	}

	std::shared_ptr<pxe::label> title;
	if(const auto err = get_component<pxe::label>(title_label_).unwrap(title); err) {
		return pxe::error("failed to get title label", *err);
	}
	title->set_text("Select Cosmic Difficulty");
	title->set_font_size(32);
	title->set_centered(true);

	std::shared_ptr<pxe::button> normal_btn;
	if(const auto err = get_component<pxe::button>(normal_button_).unwrap(normal_btn); err) {
		return pxe::error("failed to get normal button", *err);
	}
	normal_btn->set_text(GuiIconText(ICON_HEART, "Normal"));
	normal_btn->set_size({.width = 120, .height = 40});
	normal_btn->set_font_size(25);
	normal_btn->set_controller_button(GAMEPAD_BUTTON_RIGHT_FACE_DOWN);

	std::shared_ptr<pxe::button> hard_btn;
	if(const auto err = get_component<pxe::button>(hard_button_).unwrap(hard_btn); err) {
		return pxe::error("failed to get hard button", *err);
	}
	hard_btn->set_text(GuiIconText(ICON_PLAYER_JUMP, "Hard"));
	hard_btn->set_size({.width = 120, .height = 40});
	hard_btn->set_font_size(25);
	hard_btn->set_controller_button(GAMEPAD_BUTTON_RIGHT_FACE_UP);

	std::shared_ptr<pxe::button> burger_daddy_btn;
	if(const auto err = get_component<pxe::button>(burger_daddy_button_).unwrap(burger_daddy_btn); err) {
		return pxe::error("failed to get burger daddy button", *err);
	}
	burger_daddy_btn->set_text(GuiIconText(ICON_DEMON, "Burger Daddy"));
	burger_daddy_btn->set_size({.width = 190, .height = 40});
	burger_daddy_btn->set_font_size(25);
	burger_daddy_btn->set_controller_button(GAMEPAD_BUTTON_RIGHT_FACE_LEFT);

	std::shared_ptr<pxe::button> back_btn;
	if(const auto err = get_component<pxe::button>(back_button_).unwrap(back_btn); err) {
		return pxe::error("failed to get back button", *err);
	}
	back_btn->set_text(GuiIconText(ICON_PLAYER_PREVIOUS, "Back"));
	back_btn->set_size({.width = 70, .height = 25});
	back_btn->set_font_size(button_font_size);
	back_btn->set_controller_button(GAMEPAD_BUTTON_RIGHT_FACE_RIGHT);

	button_click_ = app.bind_event<pxe::button::click>(this, &cosmic::on_button_click);
	return true;
}

auto cosmic::end() -> pxe::result<> {
	get_app().unsubscribe(button_click_);
	return scene::end();
}

auto cosmic::update(float delta) -> pxe::result<> {
	return scene::update(delta);
}

auto cosmic::show() -> pxe::result<> {
	return scene::show();
}

auto cosmic::layout(const pxe::size screen_size) -> pxe::result<> {
	std::shared_ptr<pxe::button> classic_btn;
	if(const auto err = get_component<pxe::button>(normal_button_).unwrap(classic_btn); err) {
		return pxe::error("failed to get normal button", *err);
	}
	std::shared_ptr<pxe::button> normal_btn;
	if(const auto err = get_component<pxe::button>(hard_button_).unwrap(normal_btn); err) {
		return pxe::error("failed to get hard button", *err);
	}
	std::shared_ptr<pxe::button> burger_daddy_btn;
	if(const auto err = get_component<pxe::button>(burger_daddy_button_).unwrap(burger_daddy_btn); err) {
		return pxe::error("failed to get burger daddy button", *err);
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
	const auto [cow, coh] = normal_btn->get_size();
	const auto [bdw, bdh] = burger_daddy_btn->get_size();
	const auto [bca, bch] = back_btn->get_size();

	constexpr auto button_v_gap = 20.0F;
	title->set_position({
		.x = screen_size.width / 2.0F,
		.y = button_v_gap,
	});

	constexpr float gap = 40.0F;

	const float total_width = clw + gap + cow + gap + bdw;
	const float start_x = (screen_size.width - total_width) / 2.0F;
	const float y = screen_size.height / 2.0F;

	classic_btn->set_position({.x = start_x, .y = y});
	normal_btn->set_position({.x = start_x + clw + gap, .y = y});
	burger_daddy_btn->set_position({.x = start_x + clw + gap + cow + gap, .y = y});

	back_btn->set_position({
		.x = (screen_size.width - bca) / 2.0F,
		.y = screen_size.height - bch - button_v_gap,
	});

	return true;
}

auto cosmic::on_button_click(const pxe::button::click &evt) -> pxe::result<> {
	if(evt.id == normal_button_) {
		get_app().post_event(selected{.difficulty = level_manager::difficulty::normal});
	} else if(evt.id == hard_button_) {
		get_app().post_event(selected{.difficulty = level_manager::difficulty::hard});
	} else if(evt.id == burger_daddy_button_) {
		get_app().post_event(selected{.difficulty = level_manager::difficulty::burger_daddy});
	} else if(evt.id == back_button_) {
		get_app().post_event(back{});
	}
	return true;
}

} // namespace energy
