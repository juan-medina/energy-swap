// SPDX-FileCopyrightText: 2026 Juan Medina
// SPDX-License-Identifier: MIT

#pragma once

#include <pxe/app.hpp>
#include <pxe/components/button.hpp>
#include <pxe/components/component.hpp>
#include <pxe/components/label.hpp>
#include <pxe/result.hpp>
#include <pxe/scenes/scene.hpp>

#include "../level_manager.hpp"

#include <cstddef>

namespace energy {

// =============================================================================
// cosmic Scene Declaration
// =============================================================================

class cosmic: public pxe::scene {
public:
	cosmic() = default;
	~cosmic() override = default;

	// Copyable
	cosmic(const cosmic &) = default;
	auto operator=(const cosmic &) -> cosmic & = default;
	// Movable
	cosmic(cosmic &&) noexcept = default;
	auto operator=(cosmic &&) noexcept -> cosmic & = default;

	[[nodiscard]] auto init(pxe::app &app) -> pxe::result<> override;
	[[nodiscard]] auto end() -> pxe::result<> override;
	[[nodiscard]] auto update(float delta) -> pxe::result<> override;
	auto layout(pxe::size screen_size) -> pxe::result<> override;
	[[nodiscard]] auto show() -> pxe::result<> override;

	struct back {};
	struct selected {
		level_manager::difficulty difficulty;
	};

private:
	// =============================================================================
	// Constants
	// =============================================================================
	static constexpr auto button_font_size = 16;

	// =============================================================================
	// Components
	// =============================================================================
	size_t title_label_{};
	size_t normal_button_{};
	size_t hard_button_{};
	size_t burger_daddy_button_{};
	size_t back_button_{};
	int button_click_{};

	// =============================================================================
	// Event Handlers
	// =============================================================================
	[[nodiscard]] auto on_button_click(const pxe::button::click &evt) -> pxe::result<>;
};

} // namespace energy
