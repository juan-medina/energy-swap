// SPDX-FileCopyrightText: 2026 Juan Medina
// SPDX-License-Identifier: MIT

#pragma once

#include "ui_component.hpp"

namespace engine {

class label: public ui_component {
public:
	label() = default;
	~label() override = default;

	// Non-copyable
	label(const label &) = delete;
	auto operator=(const label &) -> label & = delete;

	// Non-movable
	label(label &&) noexcept = delete;
	auto operator=(label &&) noexcept -> label & = delete;

	[[nodiscard]] auto init(app &app) -> result<> override;
	[[nodiscard]] auto end() -> result<> override;

	[[nodiscard]] auto update(float delta) -> result<> override;
	[[nodiscard]] auto draw() -> result<> override;

	auto set_text(const std::string &text) -> void;

	[[nodiscard]] auto get_text() const -> const std::string & {
		return text_;
	}

	auto set_font_size(const float &size) -> void override;

private:
	std::string text_{"label"};

	auto calculate_size() -> void;
};

} // namespace engine

