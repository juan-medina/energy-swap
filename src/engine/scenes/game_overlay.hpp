// SPDX-FileCopyrightText: 2026 Juan Medina
// SPDX-License-Identifier: MIT

#pragma once

#include "../components/component.hpp"
#include "../result.hpp"
#include "scene.hpp"

#include <cstddef>
#include <string>

namespace engine {
class app;
struct size;

class game_overlay: public scene {
public:
	[[nodiscard]] auto init(app &app) -> result<> override;
	[[nodiscard]] auto end() -> result<> override;

	auto layout(size screen_size) -> result<> override;

	struct version_click {};

private:
	size_t version_display_ = 0;
	static constexpr auto margin = 10.0F;
	int click_{0};
};

} // namespace engine