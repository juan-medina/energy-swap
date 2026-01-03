// SPDX-FileCopyrightText: 2026 Juan Medina
// SPDX-License-Identifier: MIT

#pragma once

#include "ui_component.hpp"

#include <string>

namespace engine {

class button: public ui_component {
public:
	button();
	~button() override = default;

	// Non-copyable
	button(const button &) = delete;
	auto operator=(const button &) -> button & = delete;

	// Non-movable
	button(button &&) noexcept = delete;
	auto operator=(button &&) noexcept -> button & = delete;

	[[nodiscard]] auto init(app &app) -> result<> override;
	[[nodiscard]] auto end() -> result<> override;

	[[nodiscard]] auto update(float delta) -> result<> override;
	[[nodiscard]] auto draw() -> result<> override;

	auto set_text(const std::string &text) -> void {
		text_ = text;
	}

	[[nodiscard]] auto get_text() const -> const std::string & {
		return text_;
	}

	[[nodiscard]] auto get_id() const -> int {
		return id_;
	}

	struct click {
		int id{};
	};

private:
	static int next_id;

	std::string text_{"Button"};
	int id_{0};
};

} // namespace engine