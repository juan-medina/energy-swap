// SPDX-FileCopyrightText: 2026 Juan Medina
// SPDX-License-Identifier: MIT

#include "level_manager.hpp"

#include <format>
#include <spdlog/spdlog.h>
#include <sstream>

namespace energy {

auto level_manager::load_levels(const std::string &levels_path) -> pxe::result<> {
	levels_.clear();
	auto *text = LoadFileText(levels_path.c_str());
	if(text == nullptr) {
		return pxe::error(std::format("failed to load levels file from {}", levels_path));
	}
	auto stream = std::istringstream(text);
	auto line = std::string{};
	while(std::getline(stream, line)) {
		levels_.emplace_back(line);
	}
	UnloadFileText(text);
	if(levels_.empty()) {
		return pxe::error(std::format("no levels found in file {}", levels_path));
	}
	SPDLOG_DEBUG("loaded {} levels from {}", levels_.size(), levels_path);
	return true;
}

auto level_manager::get_current_level_string() const -> const std::string & {
	return levels_.at(current_level_ - 1);
}

auto level_manager::get_total_levels() const -> size_t {
	return levels_.size();
}

} // namespace energy
