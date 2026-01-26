// SPDX-FileCopyrightText: 2026 Juan Medina
// SPDX-License-Identifier: MIT

#pragma once

#include <pxe/result.hpp>

#include "battery.hpp"

#include <cstddef>
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
	[[nodiscard]] auto solve() const -> std::vector<energy::puzzle::move>;

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

	[[nodiscard]] auto is_solved() const -> bool;

	[[nodiscard]] auto is_solvable() const -> bool;

private:
	std::vector<battery> batteries_;
	static constexpr auto max_batteries = 12;
	[[nodiscard]] static auto generate_moves(const puzzle &state) -> std::vector<move>;
	static auto push_next_moves(const puzzle &state,
								const std::vector<move> &moves,
								std::vector<std::pair<puzzle, std::vector<move>>> &stack) -> void;
};

} // namespace energy
