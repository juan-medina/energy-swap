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
// Mode Scene Declaration
// =============================================================================

class mode: public pxe::scene {
public:
	mode() = default;
	~mode() override = default;

	// Copyable
	mode(const mode &) = default;
	auto operator=(const mode &) -> mode & = default;
	// Movable
	mode(mode &&) noexcept = default;
	auto operator=(mode &&) noexcept -> mode & = default;

	[[nodiscard]] auto init(pxe::app &app) -> pxe::result<> override;
	[[nodiscard]] auto end() -> pxe::result<> override;
	[[nodiscard]] auto update(float delta) -> pxe::result<> override;
	auto layout(pxe::size screen_size) -> pxe::result<> override;
	[[nodiscard]] auto show() -> pxe::result<> override;

	struct back {};
	struct selected {
		level_manager::mode mode;
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
	size_t classic_button_{};
	size_t cosmic_button_{};
	size_t cosmic_label_error{};
	size_t back_button_{};
	int button_click_{};

	// =============================================================================
	// Event Handlers
	// =============================================================================
	[[nodiscard]] auto on_button_click(const pxe::button::click &evt) -> pxe::result<>;
};

} // namespace energy
