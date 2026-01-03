// SPDX-FileCopyrightText: 2026 Juan Medina
// SPDX-License-Identifier: MIT

#include "game_overlay.hpp"

#include "../app.hpp"
#include "../result.hpp"

#include <raylib.h>

#ifdef _WIN32
#	include <windows.h>

#	include <shellapi.h>

#elif defined(__APPLE__) || defined(__linux__)
#	include <unistd.h>
#elif defined(__EMSCRIPTEN__)
#	include <emscripten/emscripten.h>
#	include <emscripten/val.h>
#endif

namespace engine {

auto game_overlay::init(app &app) -> result<> {
	// store app reference in base component
	if(const auto err = scene::init(app).ko(); err) {
		return error("Failed to initialize base component", *err);
	}

	if(const auto err = version_display_.init(app).ko(); err) {
		return error("Failed to initialize version display", *err);
	}

	using click = version_display::click;
	// use provided app reference for subscription; base stored app_ will be used elsewhere
	click_ = app.subscribe<click>([](const click &) -> void { open_url("https://juan-medina.com"); });

	return true;
}

auto game_overlay::end() -> result<> {
	// unsubscribe using stored app_ (inherited from component) and call base end
	if(app_.has_value()) {
		app_->get().unsubscribe(click_);
	}
	if(const auto err = version_display_.end().ko(); err) {
		return error("Failed to end version display", *err);
	}
	return scene::end();
}

auto game_overlay::update(const float delta) -> result<> {
	if(const auto err = version_display_.update(delta).ko(); err) {
		return error("Failed to update version display", *err);
	}
	return true;
}

auto game_overlay::draw() -> result<> {
	if(const auto err = version_display_.draw().ko(); err) {
		return error("Failed to draw version display", *err);
	}
	return true;
}

auto game_overlay::layout(const Vector2 screen_size) -> void {
	// position version display at bottom-right corner with margin
	const auto [width, height] = version_display_.get_size();
	version_display_.set_position({
		.x = screen_size.x - width - margin,
		.y = screen_size.y - height - margin,
	});
}

auto game_overlay::open_url(const std::string &url) -> result<> {
#ifdef _WIN32
	if(auto *result = ShellExecuteA(nullptr, "open", url.c_str(), nullptr, nullptr, 1);
	   reinterpret_cast<intptr_t>(result) <= 32) { // NOLINT(*-pro-type-reinterpret-cast)
		return error("Failed to open URL using shell execute");
	}
	return true;
#elif defined(__APPLE__) || defined(__linux__)
#	ifdef __APPLE__
	const std::string open_command = "open";
#	else
	const std::string open_command = "xdg-open";
#	endif
	const auto pid = fork();
	if(pid == 0) {
		std::vector cmd(open_command.begin(), open_command.end());
		cmd.push_back('\0');
		std::vector arg(url.begin(), url.end());
		arg.push_back('\0');
		const std::vector<char *> argv{cmd.data(), arg.data(), nullptr};
		execvp(cmd.data(), argv.data());
		_exit(1);
	}
	if(pid > 0) {
		return true;
	}
	return error("Failed to fork process to open URL");
#elif defined(__EMSCRIPTEN__)
	using emscripten::val;

	const auto document = val::global("document");
	auto anchor = document.call<val>("createElement", val("a"));
	anchor.set("href", url);
	anchor.set("target", "_blank");
	anchor.set("rel", "noopener noreferrer");
	const auto body_list = document.call<val>("getElementsByTagName", val("body"));
	const auto body = body_list.call<val>("item", val(0));

	body.call<void>("appendChild", anchor);
	anchor.call<void>("click");
	body.call<void>("removeChild", anchor);

	return true;
#endif
}

} // namespace engine