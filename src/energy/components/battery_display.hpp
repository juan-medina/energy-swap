// SPDX-FileCopyrightText: 2026 Juan Medina
// SPDX-License-Identifier: MIT

#pragma once

#include "../../engine/components/sprite.hpp"
#include "../data/battery.hpp"

namespace energy {

class battery_display: public engine::sprite {
public:
	auto init(engine::app &app) -> engine::result<> override;

	static constexpr auto sprite_sheet_name = "sprites";
	static constexpr auto frame = "battery.png";

	auto set_battery(battery &bat) -> void {
		battery_ = bat;
	}

	[[nodiscard]] auto draw() -> engine::result<> override;

private:
	std::optional<std::reference_wrapper<battery>> battery_;
};

} // namespace energy
