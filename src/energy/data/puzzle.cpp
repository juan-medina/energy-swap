// SPDX-FileCopyrightText: 2026 Juan Medina
// SPDX-License-Identifier: MIT

#include "puzzle.hpp"

#include "spdlog/spdlog.h"

#include <algorithm>
#include <ranges>

namespace energy {

auto puzzle::from_string(const std::string &str) -> engine::result<puzzle> {
	puzzle result;

	// split by -, first part is the batteries, second we ignore for now
	const auto delimiter_pos = str.find('-');
	if(delimiter_pos == std::string::npos) {
		return engine::error("invalid puzzle string format, missing delimiter");
	}
	const auto batteries_str = str.substr(0, delimiter_pos);

	if(batteries_str.empty()) {
		return engine::error("battery string is empty");
	}

	if(batteries_str.size() / 4 > max_batteries) {
		return engine::error("too many batteries in puzzle string");
	}
	if(batteries_str.size() % 4 != 0) {
		return engine::error("invalid battery string length, must be multiple of 4");
	}

	for(size_t i = 0; i < batteries_str.size(); i += 4) {
		const auto battery_str = batteries_str.substr(i, 4);
		const auto [battery, error] = battery::from_string(battery_str).ok();
		if(error) {
			return engine::error("failed to parse battery in puzzle from string", *error);
		}

		result.batteries_.push_back(*battery);
	}

	return result;
}

auto puzzle::is_solved() const -> bool {
	return std::ranges::all_of(batteries_,
							   [](const auto &battery) -> auto { return battery.closed() || battery.empty(); });
}

auto puzzle::is_solvable() const -> bool {
	return std::ranges::any_of(batteries_, [this](const auto &from_bat) -> auto {
		if(from_bat.closed() || from_bat.empty()) {
			return false;
		}
		return std::ranges::any_of(batteries_, [&from_bat](const auto &to_bat) -> auto {
			if(std::addressof(from_bat) == std::addressof(to_bat) || to_bat.closed() || to_bat.full()) {
				return false;
			}
			return to_bat.can_get_from(from_bat);
		});
	});
}

} // namespace energy