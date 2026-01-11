// SPDX-FileCopyrightText: 2026 Juan Medina
// SPDX-LicenseCopyrightText: MIT

#pragma once

#include "../result.hpp"
#include "component.hpp"
#include "sprite.hpp"

#include <raylib.h>

#include <cstddef>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace engine {

class quick_bar: public component {
public:
	quick_bar() = default;
	~quick_bar() override = default;

	// Copyable
	quick_bar(const quick_bar &) = default;
	auto operator=(const quick_bar &) -> quick_bar & = default;

	// Movable
	quick_bar(quick_bar &&) noexcept = default;
	auto operator=(quick_bar &&) noexcept -> quick_bar & = default;

	[[nodiscard]] auto init(app &app, const std::string &sprite_sheet, float gap = 0.0F) -> result<>;
	auto set_position(const Vector2 &pos) -> void override;
	[[nodiscard]] auto end() -> result<> override;
	[[nodiscard]] auto update(float delta) -> result<> override;
	[[nodiscard]] auto draw() -> result<> override;

	[[nodiscard]] auto add_sprite(const std::string &frame_name) -> result<size_t>;

private:
	auto recalculate() -> void;
	auto recalculate_size() -> void;

	float gap_{0.0F};
	std::string sprite_sheet_;
	std::vector<std::shared_ptr<sprite>> sprites_;
};

} // namespace engine
