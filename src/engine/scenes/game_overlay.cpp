// SPDX-FileCopyrightText: 2026 Juan Medina
// SPDX-License-Identifier: MIT

#include "game_overlay.hpp"

#include "../app.hpp"
#include "../components/component.hpp"
#include "../components/version_display.hpp"
#include "../result.hpp"
#include "scene.hpp"

#include <functional>
#include <memory>
#include <optional>

#ifdef _WIN32
#	include <shellapi.h>
#	include <cstdint>
#elif defined(__APPLE__) || defined(__linux__)
#	include <unistd.h>
#	include <vector>
#elif defined(__EMSCRIPTEN__)
#	include <emscripten/emscripten.h>
#	include <emscripten/val.h>
#endif

namespace engine {

auto game_overlay::init(app &app) -> result<> {
	if(const auto err = scene::init(app).ko(); err) {
		return error("failed to initialize base component", *err);
	}

	const auto [id, err] = register_component<version_display>().ok();
	if(err) {
		return error("failed to register version display component", *err);
	}
	version_display_ = *id;

	using click = version_display::click;
	click_ = get_app().subscribe<click>([](const click &) -> result<> {
		open_url("https://juan-medina.com");
		return true;
	});

	return true;
}

auto game_overlay::end() -> result<> {
	get_app().unsubscribe(click_);
	return scene::end();
}

auto game_overlay::layout(const size screen_size) -> result<> {
	auto [version, err] = get_component<version_display>(version_display_).ok();
	if(err) {
		return error("failed to get version display component", *err);
	}

	// position version display at bottom-right corner with margin
	const auto [width, height] = (*version)->get_size();
	(*version)->set_position({
		.x = screen_size.width - width - margin,
		.y = screen_size.height - height - margin,
	});

	return true;
}

auto game_overlay::open_url(const std::string &url) -> result<> {
#ifdef _WIN32
	if(auto *result = ShellExecuteA(nullptr, "open", url.c_str(), nullptr, nullptr, 1);
	   reinterpret_cast<intptr_t>(result) <= 32) { // NOLINT(*-pro-type-reinterpret-cast)
		return error("failed to open URL using shell execute");
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
	return error("failed to fork process to open URL");
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