// SPDX-FileCopyrightText: 2026 Juan Medina
// SPDX-License-Identifier: MIT

#pragma once

#include <pxe/components/label.hpp>
#include <pxe/result.hpp>

#include <raylib.h>

namespace pxe {
class app;
} // namespace pxe

namespace energy {

class points: public pxe::label {
public:
	auto set_points(int points) -> void;
	auto update(float delta_time) -> pxe::result<> override;

private:
	static constexpr auto time_to_live = 2.0F; // seconds
	static constexpr auto up_speed = 40.0F;	   // pixels per second
	static constexpr auto fade_green = Color{.r = 0, .g = 255, .b = 0, .a = 0};
	static constexpr auto full_green = Color{.r = 0, .g = 255, .b = 0, .a = 255};

	float time_to_live_{0.0F};
};

} // namespace energy