// SPDX-FileCopyrightText: 2026 Juan Medina
// SPDX-License-Identifier: MIT

#include "battery_display.hpp"

#include <spdlog/spdlog.h>

namespace energy {

auto battery_display::init(engine::app &app) -> engine::result<> {
	return sprite::init(app, sprite_sheet_name, frame);
}

auto battery_display::draw() -> engine::result<> {
	if(!is_visible()) {
		return true;
	}
	assert(battery_.has_value() && "Battery reference not set for battery display");
	const auto result = sprite::draw();

	const auto &bat = battery_->get();
	const auto test_text = std::format("{}", bat.size());

	DrawText(test_text.c_str(), static_cast<int>(get_pos().x), static_cast<int>(get_pos().y), 20, BLACK);
	return result;
}

} // namespace energy