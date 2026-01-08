// SPDX-FileCopyrightText: 2026 Juan Medina
// SPDX-License-Identifier: MIT

#pragma once

#include "../components/component.hpp"
#include "../result.hpp"
#include "texture.hpp"

#include <raylib.h>

#include <filesystem>
#include <jsoncons/basic_json.hpp>
#include <jsoncons/json.hpp>
#include <string>
#include <system_error>
#include <unordered_map>

namespace engine {

class sprite_sheet {
public:
	explicit sprite_sheet() = default;
	virtual ~sprite_sheet() = default;

	// Non-copyable
	sprite_sheet(const sprite_sheet &) = delete;
	auto operator=(const sprite_sheet &) -> sprite_sheet & = delete;

	// Movable
	sprite_sheet(sprite_sheet &&) noexcept = default;
	auto operator=(sprite_sheet &&) noexcept -> sprite_sheet & = default;

	auto init(const std::string &path) -> result<>;
	auto end() -> result<>;
	[[nodiscard]] auto
	draw(const std::string &frame_name, const Vector2 &pos, const float &scale, const Color &tint = WHITE) const
		-> result<>;

	[[nodiscard]] auto frame_size(const std::string &frame_name) const -> result<size>;

	[[nodiscard]] auto frame_pivot(const std::string &frame_name) const -> result<Vector2>;

private:
	struct frame {
		Rectangle origin{};
		Vector2 pivot{};
	};

	texture texture_;
	std::unordered_map<std::string, frame> frames_;

	auto parse_frames(const jsoncons::json &parser) -> result<>;
	auto parse_meta(const jsoncons::json &parser, const std::filesystem::path &base_path) -> result<>;
};

} // namespace engine