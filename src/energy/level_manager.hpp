// SPDX-FileCopyrightText: 2026 Juan Medina
// SPDX-License-Identifier: MIT

#pragma once

#include <pxe/result.hpp>

#include <cstddef>
#include <string>
#include <vector>

namespace energy {

class level_manager {
public:
	level_manager() = default;
	~level_manager() = default;

	// Non-copyable, non-movable
	level_manager(const level_manager &) = delete;
	auto operator=(const level_manager &) -> level_manager & = delete;
	level_manager(level_manager &&) = delete;
	auto operator=(level_manager &&) -> level_manager & = delete;

	auto load_levels(const std::string &levels_path) -> pxe::result<>;

	auto set_current_level(size_t level) -> void {
		current_level_ = level;
	}
	[[nodiscard]] auto get_current_level() const -> size_t {
		return current_level_;
	}

	[[nodiscard]] auto get_current_level_string() const -> const std::string &;
	[[nodiscard]] auto get_total_levels() const -> size_t;

	// For normal mode
	[[nodiscard]] auto get_max_reached_level() const -> size_t {
		return max_reached_level_;
	}
	auto set_max_reached_level(size_t level) -> void {
		max_reached_level_ = level;
	}

	[[nodiscard]] auto can_have_solution_hint() const -> bool {
		return current_level_ <= 5;
	}

private:
	std::vector<std::string> levels_;
	size_t current_level_ = 1;
	size_t max_reached_level_ = 1;
};

} // namespace energy
