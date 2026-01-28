// SPDX-FileCopyrightText: 2026 Juan Medina
// SPDX-License-Identifier: MIT

#include "level_manager.hpp"

#include <pxe/result.hpp>

#include "data/puzzle.hpp"

#include <cstddef>
#include <cstdint>
#include <format>
#include <fstream>
#include <jsoncons/basic_json.hpp>
#include <jsoncons/json_decoder.hpp>
#include <jsoncons/json_reader.hpp>
#include <spdlog/spdlog.h>
#include <sstream>
#include <string>
#include <system_error>

namespace energy {

auto level_manager::load_classic_levels(const std::string &levels_path) -> pxe::result<> {
	classic_levels_.clear();
	std::ifstream const file(levels_path);
	if(!file.is_open()) {
		return pxe::error(std::format("failed to open levels json file: {}", levels_path));
	}
	std::stringstream buffer;
	buffer << file.rdbuf();
	std::error_code error_code;
	jsoncons::json_decoder<jsoncons::json> decoder;
	jsoncons::json_stream_reader reader(buffer, decoder);
	reader.read(error_code);
	if(error_code) {
		return pxe::error(std::format("JSON parse error: {}", error_code.message()));
	}
	const auto &parsed = decoder.get_result();
	if(!parsed.is_array()) {
		return pxe::error("levels.json root is not an array");
	}
	for(const auto &level: parsed.array_range()) {
		if(!level.contains("puzzle")
		   || !level["puzzle"].is_string()) { // NOLINT(*-pro-bounds-avoid-unchecked-container-access)
			return pxe::error("level entry missing 'puzzle' string");
		}
		classic_levels_.emplace_back(
			level["puzzle"].as<std::string>()); // NOLINT(*-pro-bounds-avoid-unchecked-container-access)
	}
	if(classic_levels_.empty()) {
		return pxe::error(std::format("no levels found in file {}", levels_path));
	}
	SPDLOG_DEBUG("loaded {} levels from {} (json)", classic_levels_.size(), levels_path);
	return true;
}

auto level_manager::load_cosmic_levels(const std::string &levels_path) -> pxe::result<> {
	cosmic_levels_.clear();
	std::ifstream const file(levels_path);
	if(!file.is_open()) {
		return pxe::error(std::format("failed to open cosmic levels json file: {}", levels_path));
	}
	std::stringstream buffer;
	buffer << file.rdbuf();
	std::error_code error_code;
	jsoncons::json_decoder<jsoncons::json> decoder;
	jsoncons::json_stream_reader reader(buffer, decoder);
	reader.read(error_code);
	if(error_code) {
		return pxe::error(std::format("JSON parse error: {}", error_code.message()));
	}
	const auto &parsed = decoder.get_result();
	if(!parsed.is_array()) {
		return pxe::error("cosmic.json root is not an array");
	}
	for(const auto &level: parsed.array_range()) {
		if(!level.contains("difficult")
		   || !level["difficult"].is_int64()) { // NOLINT(*-pro-bounds-avoid-unchecked-container-access)
			return pxe::error("cosmic level entry missing 'difficult' int");
		}
		if(!level.contains("ranges")
		   || !level["ranges"].is_array()) { // NOLINT(*-pro-bounds-avoid-unchecked-container-access)
			return pxe::error("cosmic level entry missing 'ranges' array");
		}
		cosmic_level cosmic{};
		const auto diff_val = level["difficult"].as<int64_t>(); // NOLINT(*-pro-bounds-avoid-unchecked-container-access)
		if(diff_val < 0 || diff_val > 2) {
			return pxe::error("cosmic level 'difficult' value out of range");
		}
		cosmic.difficult = static_cast<difficulty>(diff_val);
		for(const auto &range: level["ranges"].array_range()) { // NOLINT(*-pro-bounds-avoid-unchecked-container-access)
			if(!range.contains("from")
			   || !range["from"].is_uint64() // NOLINT(*-pro-bounds-avoid-unchecked-container-access)
			   || !range.contains("to")
			   || !range["to"].is_uint64() // NOLINT(*-pro-bounds-avoid-unchecked-container-access)
			   || !range.contains("energies")
			   || !range["energies"].is_uint64() // NOLINT(*-pro-bounds-avoid-unchecked-container-access)
			   || !range.contains("empty")
			   || !range["empty"].is_uint64()) { // NOLINT(*-pro-bounds-avoid-unchecked-container-access)
				return pxe::error("cosmic range entry missing required uint fields");
			}
			cosmic_range r{};
			r.from = range["from"].as<uint64_t>();		   // NOLINT(*-pro-bounds-avoid-unchecked-container-access)
			r.to = range["to"].as<uint64_t>();			   // NOLINT(*-pro-bounds-avoid-unchecked-container-access)
			r.energies = range["energies"].as<uint64_t>(); // NOLINT(*-pro-bounds-avoid-unchecked-container-access)
			r.empty = range["empty"].as<uint64_t>();	   // NOLINT(*-pro-bounds-avoid-unchecked-container-access)
			cosmic.ranges.push_back(r);
		}
		cosmic_levels_.push_back(cosmic);
	}
	if(cosmic_levels_.empty()) {
		return pxe::error(std::format("no cosmic levels found in file {}", levels_path));
	}
	SPDLOG_DEBUG("loaded {} cosmic levels from {} (json)", cosmic_levels_.size(), levels_path);
	return true;
}

auto level_manager::load_levels() -> pxe::result<> {
	if(const auto err = load_classic_levels(classic_levels_path).unwrap(); err) {
		return pxe::error("failed to load classic levels", *err);
	}
	if(const auto err = load_cosmic_levels(cosmic_levels_path).unwrap(); err) {
		return pxe::error("failed to load cosmic levels", *err);
	}
	return true;
}

auto level_manager::get_current_level_string() const -> pxe::result<std::string> {
	if(current_mode_ == mode::cosmic) {
		for(const auto &[difficult, ranges]: cosmic_levels_) {
			if(difficult == current_difficulty_) {
				for(const auto &[from, to, energies, empty]: ranges) {
					if(current_level_ >= from && current_level_ <= to) {
						return generate_cosmic_level_string(energies, empty);
					}
				}
			}
		}
		return pxe::error("cosmic level entry missing cosmic levels");
	}
	return classic_levels_.at(current_level_ - 1);
}

auto level_manager::get_total_levels() const -> size_t {
	return classic_levels_.size();
}

auto level_manager::generate_cosmic_level_string(const size_t energies, const size_t empty) -> std::string {
	while(true) {
		const auto new_puzzle = puzzle::random(energies, empty); // generate a random puzzle
		if(new_puzzle.has_any_full_battery()) {					 // avoid puzzles with any full battery
			continue;
		}
		if(const auto solution = new_puzzle.solve(false); !solution.empty()) { // ensure the puzzle is solvable
			return new_puzzle.to_string();
		}
	}
}

} // namespace energy
