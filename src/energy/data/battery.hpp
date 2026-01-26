// SPDX-FileCopyrightText: 2026 Juan Medina
// SPDX-License-Identifier: MIT

#pragma once

#include <pxe/result.hpp>

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace energy {

class battery {
public:
	static constexpr int max_energy = 4;
	static constexpr int max_energy_types = 10;

	[[nodiscard]] auto full() const -> bool {
		return current_state_ == state::full;
	}
	[[nodiscard]] auto closed() const -> bool {
		return current_state_ == state::closed;
	}
	[[nodiscard]] auto empty() const -> bool {
		return current_state_ == state::empty;
	}
	[[nodiscard]] auto size() const -> int {
		return static_cast<int>(energies_.size());
	}

	void add(int energy_type);
	void remove();

	[[nodiscard]] auto top() const -> std::vector<int>;

	[[nodiscard]] auto can_get_from(const battery &other) const -> bool;

	auto transfer_energy_from(battery &other) -> void;

	[[nodiscard]] auto at(const size_t index) const -> int {
		if(index >= energies_.size()) {
			return 0;
		}
		return energies_.at(index);
	}

	[[nodiscard]] auto string() const -> std::string;
	[[nodiscard]] static auto from_string(const std::string &str) -> pxe::result<battery>;

private:
	enum class state : std::uint8_t { normal, empty, full, closed };
	std::vector<int> energies_;
	state current_state_ = state::empty;
};

} // namespace energy
