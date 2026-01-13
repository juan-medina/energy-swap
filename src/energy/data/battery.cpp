// SPDX-FileCopyrightText: 2026 Juan Medina
// SPDX-License-Identifier: MIT

#include "battery.hpp"

#include <pxe/result.hpp>

#include <algorithm>
#include <cassert>
#include <format>
#include <functional>
#include <string>
#include <vector>

namespace energy {

void battery::add(const int energy_type) {
	assert(energy_type > 0 && energy_type <= max_energy_types && "Invalid energy type");
	assert(current_state_ != state::closed && "Cannot add energy to a closed battery");
	assert(current_state_ != state::full && "Cannot add energy to a full battery");
	energies_.push_back(energy_type);
	if(energies_.size() < max_energy) {
		current_state_ = state::normal;
	} else {
		const int first = energies_.front();
		const bool all_same = std::ranges::all_of(energies_, [&](const int energy) -> bool { return energy == first; });
		current_state_ = all_same ? state::closed : state::full;
	}
}

void battery::remove() {
	assert(current_state_ != state::empty && "Cannot remove energy from an empty battery");
	assert(current_state_ != state::closed && "Cannot remove energy from a closed battery");
	energies_.pop_back();
	current_state_ = energies_.empty() ? state::empty : state::normal;
}

auto battery::top() const -> std::vector<int> {
	if(energies_.empty()) {
		return {};
	}
	const auto last = energies_.back();
	std::vector result{last};
	for(int i = static_cast<int>(energies_.size()) - 2; i >= 0; --i) {
		if(energies_.at(i) == last) {
			result.push_back(last);
		} else {
			break;
		}
	}
	return result;
}

auto battery::can_get_from(const battery &other) const -> bool {
	// Cannot get energy if either battery is closed, if this battery is full, or if the other battery is empty
	if(closed() || other.closed() || full() || other.empty()) {
		return false;
	}
	// If this battery is empty, it can always get energy
	if(energies_.empty()) {
		return true;
	}
	const auto other_top = other.top();
	// Check if there is enough space with the other battery's top energies
	if(size() + other_top.size() > max_energy) {
		return false;
	}
	// Check if the top energy types match
	const auto current_top = top();
	return other_top.front() == current_top.front();
}

void battery::transfer_energy_from(battery &other) {
	assert(can_get_from(other) && "Cannot transfer energy from the other battery");
	for(const auto energy_type: other.top()) {
		add(energy_type);
		other.remove();
	}
}

auto battery::string() const -> std::string {
	std::string result;
	for(const auto energy: energies_) {
		result += std::format("{:X}", energy);
	}
	return std::format("{:0<{}}", result, max_energy);
}

auto battery::from_string(const std::string &str) -> pxe::result<battery> {
	battery new_battery;
	auto total = 0;
	for(const char character: str) {
		if(character == '0') {
			continue;
		}
		const int energy_type = std::stoi(std::string(1, character), nullptr, 16);
		if(energy_type > max_energy_types) {
			return pxe::error(std::format("invalid energy type in battery string: {}", character));
		}
		new_battery.add(energy_type);
		++total;
		if(total > max_energy) {
			return pxe::error(std::format("battery string has more energies than allowed: {} str: {}", total, str));
		}
	}
	return new_battery;
}

} // namespace energy