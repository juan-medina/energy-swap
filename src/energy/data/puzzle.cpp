// SPDX-FileCopyrightText: 2026 Juan Medina
// SPDX-License-Identifier: MIT

#include "puzzle.hpp"

#include <pxe/result.hpp>

#include "battery.hpp"

#include <algorithm>
#include <cstddef>
#include <optional>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

namespace energy {

auto puzzle::id() const -> std::string {
	std::vector<std::string> battery_ids;
	battery_ids.reserve(batteries_.size());
	for(const auto &bat: batteries_) {
		battery_ids.push_back(bat.string());
	}
	std::ranges::sort(battery_ids);
	std::string result;
	for(const auto &id: battery_ids) {
		result += id;
	}
	return result;
}

auto puzzle::solve() const -> std::vector<move> {
	using move_list = std::vector<move>;
	using state_key = std::string;
	std::unordered_set<state_key> visited;

	using frame = std::pair<puzzle, move_list>;
	std::vector<frame> stack;
	stack.emplace_back(*this, move_list{});

	while(!stack.empty()) {
		const auto [fst, snd] = std::move(stack.back());
		stack.pop_back();

		const auto &state = fst;
		const auto &moves = snd;

		const auto key = state.id();
		if(visited.contains(key)) {
			continue;
		}
		visited.insert(key);

		if(state.is_solved()) {
			return moves;
		}

		push_next_moves(state, moves, stack);
	}
	return {};
}

auto puzzle::from_string(const std::string &str) -> pxe::result<puzzle> {
	puzzle result;

	// split by -, first part is the batteries, second we ignore for now
	const auto delimiter_pos = str.find('-');
	if(delimiter_pos == std::string::npos) {
		return pxe::error("invalid puzzle string format, missing delimiter");
	}
	const auto batteries_str = str.substr(0, delimiter_pos);

	if(batteries_str.empty()) {
		return pxe::error("battery string is empty");
	}

	if(batteries_str.size() / 4 > max_batteries) {
		return pxe::error("too many batteries in puzzle string");
	}
	if(batteries_str.size() % 4 != 0) {
		return pxe::error("invalid battery string length, must be multiple of 4");
	}

	for(size_t i = 0; i < batteries_str.size(); i += 4) {
		const auto battery_str = batteries_str.substr(i, 4);
		battery parsed;
		if(const auto error = battery::from_string(battery_str).unwrap(parsed); error) {
			return pxe::error("failed to parse battery in puzzle from string", *error);
		}
		result.batteries_.push_back(parsed);
	}

	return result;
}

auto puzzle::is_solved() const -> bool {
	return std::ranges::all_of(batteries_,
							   [](const auto &battery) -> bool { return battery.closed() || battery.empty(); });
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

auto puzzle::generate_moves(const puzzle &state) -> std::vector<move> {
	std::vector<move> moves;
	const auto n = state.size();
	for(std::size_t from = 0; from < n; ++from) {
		const auto &from_bat = state.at(from);
		if(from_bat.closed() || from_bat.empty()) {
			continue;
		}
		for(std::size_t to = 0; to < n; ++to) {
			if(from == to) {
				continue;
			}
			const auto &to_bat = state.at(to);
			if(to_bat.closed() || to_bat.full()) {
				continue;
			}
			if(!state.at(to).can_get_from(from_bat)) {
				continue;
			}
			moves.push_back(move{.from = from, .to = to});
		}
	}
	return moves;
}

auto puzzle::push_next_moves(const puzzle &state,
							 const std::vector<move> &moves,
							 std::vector<std::pair<puzzle, std::vector<move>>> &stack) -> void {
	const auto n = state.size();
	for(size_t src = 0; src < n; ++src) {
		const auto &from = state.at(src);
		if(from.closed() || from.empty()) {
			continue;
		}
		for(size_t dst = 0; dst < n; ++dst) {
			if(src == dst) {
				continue;
			}
			const auto &to = state.at(dst);
			if(to.closed() || to.full()) {
				continue;
			}
			if(!to.can_get_from(from)) {
				continue;
			}
			puzzle next = state;
			next.at(dst).transfer_energy_from(next.at(src));
			auto next_moves = moves;
			next_moves.push_back(move{.from = src, .to = dst});
			stack.emplace_back(std::move(next), std::move(next_moves));
		}
	}
}

} // namespace energy
