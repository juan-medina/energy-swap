// SPDX-FileCopyrightText: 2026 Juan Medina
// SPDX-License-Identifier: MIT

#include "puzzle.hpp"

#include <pxe/result.hpp>

#include "battery.hpp"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <deque>
#include <optional>
#include <ranges>
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

auto puzzle::solve(bool optimized) const -> std::vector<move> {
	using move_list = std::vector<move>;
	using state_key = std::string;
	std::unordered_set<state_key> visited;

	using frame = std::pair<puzzle, move_list>;
	std::deque<frame> queue;
	queue.emplace_back(*this, move_list{});

	while(!queue.empty()) {
		frame current;
		if(optimized) {
			current = std::move(queue.front());
			queue.pop_front();
		} else {
			current = std::move(queue.back());
			queue.pop_back();
		}

		const auto &state = current.first;
		const auto &moves = current.second;

		const auto key = state.id();
		if(visited.contains(key)) {
			continue;
		}
		visited.insert(key);

		if(state.is_solved()) {
			return moves;
		}

		push_next_moves(state, moves, queue);
	}
	return {};
}

auto puzzle::from_string(const std::string &str) -> pxe::result<puzzle> {
	if(str.empty()) {
		return pxe::error("battery string is empty");
	}

	if(str.size() / 4 > max_batteries) {
		return pxe::error("too many batteries in puzzle string");
	}

	if(str.size() % 4 != 0) {
		return pxe::error("invalid battery string length, must be multiple of 4");
	}

	puzzle result;
	for(size_t i = 0; i < str.size(); i += 4) {
		const auto battery_str = str.substr(i, 4);
		battery parsed;
		if(const auto error = battery::from_string(battery_str).unwrap(parsed); error) {
			return pxe::error("failed to parse battery in puzzle from string", *error);
		}
		result.batteries_.push_back(parsed);
	}

	return result;
}

auto puzzle::to_string() const -> std::string {
	std::string result;
	for(const auto &bat: batteries_) {
		result += bat.string();
	}
	result += "-0";
	return result;
}

auto puzzle::random(const size_t total_energies, const size_t free_slots) -> puzzle {
	auto const total_batteries = total_energies + free_slots;
	assert(total_batteries <= max_batteries && "total energies and free slots exceed maximum capacity");
	puzzle result;

	for([[maybe_unused]] auto i: std::ranges::views::iota(0U, total_batteries)) {
		result.batteries_.emplace_back();
	}

	for([[maybe_unused]] const auto energy_type: std::ranges::views::iota(0U, total_energies)) {
		auto to_drop = battery::max_energy;
		while(to_drop > 0) {
			const auto battery_index = std::rand() % total_batteries;
			auto &bat = result.batteries_.at(battery_index);
			if(bat.full()) {
				continue;
			}
			bat.add(static_cast<int>(energy_type + 1));
			to_drop--;
		}
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

auto puzzle::push_next_moves(const puzzle &state,
							 const std::vector<move> &moves,
							 std::deque<std::pair<puzzle, std::vector<move>>> &queue) -> void {
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
			queue.emplace_back(std::move(next), std::move(next_moves));
		}
	}
}

} // namespace energy
