// SPDX-FileCopyrightText: 2026 Juan Medina
// SPDX-License-Identifier: MIT

#pragma once

#include "../../engine/result.hpp"
#include "battery.hpp"

#include <vector>

namespace energy {

class puzzle {
public:
	[[nodiscard]] auto size() const -> size_t {
		return batteries_.size();
	}

	[[nodiscard]] auto at(const size_t index) -> battery & {
		return batteries_.at(index);
	}
	[[nodiscard]] auto at(const size_t index) const -> const battery & {
		return batteries_.at(index);
	}

	[[nodiscard]] static auto from_string(const std::string &str) -> engine::result<puzzle>;

	[[nodiscard]] auto is_solved() const -> bool;

	[[nodiscard]] auto is_solvable() const -> bool;

private:
	std::vector<battery> batteries_;
	static constexpr auto max_batteries = 12;
};

} // namespace energy
