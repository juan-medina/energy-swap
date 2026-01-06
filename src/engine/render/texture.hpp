// SPDX-FileCopyrightText: 2026 Juan Medina
// SPDX-License-Identifier: MIT

#pragma once

#include "../components/component.hpp"
#include "../result.hpp"

#include <raylib.h>

namespace engine {

class texture {
public:
	explicit texture() = default;
	virtual ~texture() = default;

	// Non-copyable
	texture(const texture &) = delete;
	auto operator=(const texture &) -> texture & = delete;

	// Non-movable
	texture(texture &&) noexcept = delete;
	auto operator=(texture &&) noexcept -> texture & = delete;

	[[nodiscard]] auto draw(const Vector2 &pos) const -> result<>;

	[[nodiscard]] auto draw(Rectangle origin, Rectangle dest, Color tint, float rotation, Vector2 rotation_center) const
		-> result<>;

	[[nodiscard]] virtual auto init(const std::string &path) -> result<>;
	[[nodiscard]] virtual auto end() -> result<>;

	[[nodiscard]] auto get_size() const -> size {
		return size_;
	}

private:
	size size_{.width = 0, .height = 0};
	Texture2D texture_{};
};

} // namespace engine