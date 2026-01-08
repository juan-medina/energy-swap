// SPDX-FileCopyrightText: 2026 Juan Medina
// SPDX-License-Identifier: MIT

#include "sprite_sheet.hpp"

#include "../components/component.hpp"
#include "../result.hpp"
#include "spdlog/spdlog.h"

#include <raylib.h>

#include <format>
#include <fstream>
#include <jsoncons/json.hpp>
#include <string>

namespace engine {

auto sprite_sheet::init(const std::string &path) -> result<> {
	std::ifstream const file(path);
	if(!file.is_open()) {
		return error(std::format("sprite sheet file not found: {}", path));
	}

	std::stringstream buffer;
	buffer << file.rdbuf();

	std::error_code error_code;
	jsoncons::json_decoder<jsoncons::json> decoder;
	jsoncons::json_stream_reader reader(buffer, decoder);
	reader.read(error_code);

	if(error_code) {
		return error(std::format("JSON parse error: {}", error_code.message()));
	}

	const auto &parser = decoder.get_result();

	if(auto const err = parse_frames(parser).ko(); err) {
		return error("failed to parse sprite sheet frames", *err);
	}

	if(auto const err = parse_meta(parser, std::filesystem::path(path).parent_path()).ko(); err) {
		return error("failed to parse sprite sheet metadata", *err);
	}

	SPDLOG_DEBUG("sprite sheet : loaded from file: {}", path);

	return true;
}

auto sprite_sheet::end() -> result<> {
	if(const auto err = texture_.end().ko(); err) {
		return error("failed to end texture", *err);
	}
	return true;
}
auto sprite_sheet::draw(const std::string &frame_name, const Vector2 &pos, const float &scale, const Color &tint) const
	-> result<> {
	const auto frame_entry = frames_.find(frame_name);
	if(frame_entry == frames_.end()) {
		return error(std::format("frame not found in sprite sheet: {}", frame_name));
	}

	const auto &[origin, pivot] = frame_entry->second;
	const Rectangle destination = {
		.x = pos.x - (pivot.x * origin.width * scale),
		.y = pos.y - (pivot.y * origin.height * scale),
		.width = origin.width * scale,
		.height = origin.height * scale,
	};

	if(const auto err = texture_.draw(origin, destination, tint, 0.0F, Vector2{.x = 0.0F, .y = 0.0F}).ko(); err) {
		return error("failed to draw sprite sheet frame", *err);
	}

	return true;
}

auto sprite_sheet::frame_size(const std::string &frame_name) const -> result<size> {
	const auto frame_entry = frames_.find(frame_name);
	if(frame_entry == frames_.end()) {
		return error(std::format("frame not found in sprite sheet: {}", frame_name));
	}
	const auto &[origin, pivot] = frame_entry->second;
	return size{.width = origin.width, .height = origin.height};
}

auto sprite_sheet::frame_pivot(const std::string &frame_name) const -> result<Vector2> {
	const auto frame_entry = frames_.find(frame_name);
	if(frame_entry == frames_.end()) {
		return error(std::format("frame not found in sprite sheet: {}", frame_name));
	}
	const auto &[origin, pivot] = frame_entry->second;
	return pivot;
}

auto sprite_sheet::parse_frames(const jsoncons::json &parser) -> result<> {
	// NOLINTNEXTLINE(*-pro-bounds-avoid-unchecked-container-access)
	if(!parser.contains("frames") || !parser["frames"].is_object()) {
		return error(R"(failed to parse sprite sheet JSON: ["frames"] field missing or not an object)");
	}

	// NOLINTNEXTLINE(*-pro-bounds-avoid-unchecked-container-access)
	for(const auto &frames = parser["frames"]; const auto &frame_entry: frames.object_range()) {
		const std::string &frame_name = frame_entry.key();
		const auto &frame_object = frame_entry.value();

		// NOLINTNEXTLINE(*-pro-bounds-avoid-unchecked-container-access)
		if(!frame_object.contains("frame") && !frame_object["frame"].is_object()) {
			return error(std::format(
				R"(failed to parse sprite sheet JSON: ["frames"]["{}"]["frame"] field missing or not an object)",
				frame_name));
		}

		// NOLINTNEXTLINE(*-pro-bounds-avoid-unchecked-container-access)
		const auto &frame_data = frame_object["frame"];

		const auto origin_x = frame_data.get_value_or<float>("x", 0);
		const auto origin_y = frame_data.get_value_or<float>("y", 0);
		const auto width = frame_data.get_value_or<float>("w", 0);
		const auto height = frame_data.get_value_or<float>("h", 0);

		Rectangle const origin{
			.x = origin_x,
			.y = origin_y,
			.width = width,
			.height = height,
		};

		// NOLINTNEXTLINE(*-pro-bounds-avoid-unchecked-container-access)
		if(!frame_object.contains("pivot") || !frame_object["pivot"].is_object()) {
			return error(std::format(
				R"(failed to parse sprite sheet JSON: ["frames"]["{}"]["pivot"] field missing or not an object)",
				frame_name));
		}

		// NOLINTNEXTLINE(*-pro-bounds-avoid-unchecked-container-access)
		const auto &pivot_data = frame_object["pivot"];

		const auto pivot_x = pivot_data.get_value_or<float>("x", 0);
		const auto pivot_y = pivot_data.get_value_or<float>("y", 0);
		const Vector2 pivot{.x = pivot_x, .y = pivot_y};

		frames_.emplace(frame_name,
						frame{
							.origin = origin,
							.pivot = pivot,
						});

		SPDLOG_DEBUG("adding frame: {}", frame_name);
	}

	return true;
}

auto sprite_sheet::parse_meta(const jsoncons::json &parser, const std::filesystem::path &base_path) -> result<> {
	// NOLINTNEXTLINE(*-pro-bounds-avoid-unchecked-container-access)
	if(!parser.contains("meta") || !parser["meta"].is_object()) {
		return error(R"(failed to parse sprite sheet JSON: ["meta"] field missing or not an object)");
	}

	const auto &object = parser["meta"]; // NOLINT(*-pro-bounds-avoid-unchecked-container-access)
	const auto image = object.get_value_or<std::string>("image", "");
	if(image.empty()) {
		return error(R"(failed to parse sprite sheet JSON: ["meta"]["image"] field missing or empty)");
	}

	const auto image_path = base_path / image;

	if(const auto err = texture_.init(image_path.string()).ko(); err) {
		return error("failed to initialize texture for sprite sheet", *err);
	}

	return true;
}

} // namespace engine