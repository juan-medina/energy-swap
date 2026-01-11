// SPDX-FileCopyrightText: 2026 Juan Medina
// SPDX-License-Identifier: MIT

#pragma once

#include "../../engine/app.hpp"
#include "../../engine/components/component.hpp"
#include "../../engine/result.hpp"
#include "../../engine/scenes/scene.hpp"

#include <cstddef>

namespace energy {

class hud: public engine::scene {
public:
	[[nodiscard]] auto init(engine::app &app) -> engine::result<> override;
	[[nodiscard]] auto layout(engine::size size) -> engine::result<> override;

private:
	size_t quick_bar_{};
};

} // namespace energy
