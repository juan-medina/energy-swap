// SPDX-FileCopyrightText: 2026 Juan Medina
// SPDX-License-Identifier: MIT

#pragma once

#include <pxe/result.hpp>

#include <cstddef>
#include <cstdint>
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

	auto load_levels() -> pxe::result<>;

	auto set_current_level(const size_t level) -> void {
		current_level_ = level;
	}
	[[nodiscard]] auto get_current_level() const -> size_t {
		return current_level_;
	}

	[[nodiscard]] auto get_current_level_string() -> pxe::result<std::string>;
	[[nodiscard]] auto get_total_levels() const -> size_t;

	[[nodiscard]] auto get_max_reached_level() const -> size_t {
		return max_reached_level_;
	}
	auto set_max_reached_level(size_t level) -> void {
		max_reached_level_ = level;
	}

	[[nodiscard]] auto can_have_solution_hint() const -> bool {
		return current_mode_ == mode::classic && current_level_ <= 5;
	}

	enum class mode : std::uint8_t {
		classic,
		cosmic,
	};

	enum class difficulty : std::uint8_t {
		normal,
		hard,
		burger_daddy,
	};

	auto set_mode(const mode new_mode) -> void {
		current_mode_ = new_mode;
		last_level_string_ = 0;
		cached_level_string_.clear();
	}

	[[nodiscard]] auto get_mode() const -> mode {
		return current_mode_;
	}

	auto set_difficulty(const difficulty new_difficulty) -> void {
		current_difficulty_ = new_difficulty;
	}

	[[nodiscard]] auto get_difficulty() const -> difficulty {
		return current_difficulty_;
	}

private:
	static constexpr auto classic_levels_path = "resources/levels/classic.json";
	static constexpr auto cosmic_levels_path = "resources/levels/cosmic.json";
	std::vector<std::string> classic_levels_;

	// =============================================================================
	// Cosmic mode level data structures
	struct cosmic_range {
		size_t from;
		size_t to;
		size_t energies;
		size_t empty;
	};

	struct cosmic_level {
		difficulty difficult;
		std::vector<cosmic_range> ranges;
	};

	std::vector<cosmic_level> cosmic_levels_;

	size_t current_level_ = 1;
	size_t max_reached_level_ = 1;

	mode current_mode_{mode::classic};
	difficulty current_difficulty_{difficulty::normal};

	static auto generate_cosmic_level_string(size_t energies, size_t empty) -> std::string;
	auto load_classic_levels(const std::string &levels_path) -> pxe::result<>;
	auto load_cosmic_levels(const std::string &levels_path) -> pxe::result<>;

	size_t last_level_string_ = 0;
	std::string cached_level_string_;
};

} // namespace energy
