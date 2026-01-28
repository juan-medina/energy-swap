// SPDX-FileCopyrightText: 2026 Juan Medina
// SPDX-License-Identifier: MIT

#pragma once

#include <pxe/result.hpp>

#include "battery.hpp"

#include <algorithm>
#include <cstddef>
#include <deque>
#include <string>
#include <utility>
#include <vector>

namespace energy {

class puzzle {
public:
	// =============================================================================
	// Move representation
	struct move {
		std::size_t from;
		std::size_t to;
	};

	// =============================================================================
	// Puzzle solving and identification
	[[nodiscard]] auto id() const -> std::string;
	[[nodiscard]] auto solve(bool optimized = true) const -> std::vector<move>;

	// =============================================================================
	// Puzzle data accessors
	[[nodiscard]] auto size() const -> size_t {
		return batteries_.size();
	}

	[[nodiscard]] auto at(const size_t index) -> battery & {
		return batteries_.at(index);
	}
	[[nodiscard]] auto at(const size_t index) const -> const battery & {
		return batteries_.at(index);
	}

	// =============================================================================
	// Puzzle creation and analysis
	[[nodiscard]] static auto from_string(const std::string &str) -> pxe::result<puzzle>;
	[[nodiscard]] auto to_string() const -> std::string;
	[[nodiscard]] static auto random(size_t total_energies, size_t free_slots) -> puzzle;

	[[nodiscard]] auto is_solved() const -> bool;

	[[nodiscard]] auto is_solvable() const -> bool;

	[[nodiscard]] auto has_any_full_battery() const -> bool {
		return std::ranges::any_of(batteries_, [](const auto &bat) -> bool { return bat.full(); });
	}

private:
	std::vector<battery> batteries_;
	static constexpr auto max_batteries = 12;
	static auto push_next_moves(const puzzle &state,
								const std::vector<move> &moves,
								std::deque<std::pair<puzzle, std::vector<move>>> &queue) -> void;
};

} // namespace energy
