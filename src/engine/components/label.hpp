// SPDX-FileCopyrightText: 2026 Juan Medina
// SPDX-License-Identifier: MIT

#pragma once

#include "../app.hpp"
#include "../result.hpp"
#include "ui_component.hpp"

#include <string>

namespace engine {
class app;

class label: public ui_component {
public:
	label() = default;
	~label() override = default;

	// Copyable
	label(const label &) = default;
	auto operator=(const label &) -> label & = default;

	// Movable
	label(label &&) noexcept = default;
	auto operator=(label &&) noexcept -> label & = default;

	[[nodiscard]] auto init(app &app) -> result<> override;
	[[nodiscard]] auto end() -> result<> override;

	[[nodiscard]] auto update(float delta) -> result<> override;
	[[nodiscard]] auto draw() -> result<> override;

	auto set_text(const std::string &text) -> void;

	[[nodiscard]] auto get_text() const -> const std::string & {
		return text_;
	}

	auto set_font_size(const float &size) -> void override;

	auto set_centered(const bool centered) -> void {
		centered_ = centered;
	}

private:
	std::string text_{"label"};

	auto calculate_size() -> void;

	bool centered_{false};
};

} // namespace engine
