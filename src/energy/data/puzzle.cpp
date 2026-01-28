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
#include <random>
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
	return result;
}

auto puzzle::random(const size_t total_energies, const size_t free_slots) -> puzzle {
	const auto total_batteries = total_energies + free_slots;
	assert(total_batteries <= puzzle::max_batteries && "total energies and free slots exceed maximum capacity");

	// Step 1: Build and shuffle all possible energy types
	std::vector<int> all_types;
	all_types.reserve(battery::max_energy_types);
	for(int t = 1; t <= battery::max_energy_types; ++t) {
		all_types.push_back(t);
	}
	std::ranges::shuffle(all_types, std::mt19937{static_cast<unsigned>(std::rand())});

	// Step 2: Select the first total_energies types
	std::vector<int> chosen_types;
	chosen_types.reserve(total_energies);
	for(size_t i = 0; i < total_energies; ++i) {
		chosen_types.push_back(all_types.at(i));
	}

	// Step 3: Build a vector of all energy units using only chosen types
	std::vector<int> energies;
	energies.reserve(total_batteries * battery::max_energy);
	for(const auto type : chosen_types) {
		for(size_t i = 0; i < battery::max_energy; ++i) {
			energies.push_back(type);
		}
	}
	// Add free slots (represented as 0)
	for(size_t i = 0; i < free_slots * battery::max_energy; ++i) {
		energies.push_back(0);
	}
	// Shuffle energies
	std::ranges::shuffle(energies, std::mt19937{static_cast<unsigned>(std::rand())});

	puzzle result;
	result.batteries_.resize(total_batteries);
	// Distribute energies into batteries
	size_t idx = 0;
	for(auto &bat: result.batteries_) {
		for(size_t i = 0; i < battery::max_energy; ++i) {
			if(const auto val = energies.at(idx); val > 0) {
				bat.add(val);
			}
			++idx;
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
