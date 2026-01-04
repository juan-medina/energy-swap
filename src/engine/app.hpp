// SPDX-FileCopyrightText: 2026 Juan Medina
// SPDX-License-Identifier: MIT

#pragma once

#include "events.hpp"
#include "result.hpp"
#include "scenes/scene.hpp"

#include <raylib.h>

#include <algorithm>
#include <memory>
#include <spdlog/spdlog.h>
#include <vector>

#if defined(__GNUG__) && !defined(__EMSCRIPTEN__) && !defined(__APPLE__)
#	include <cxxabi.h>
#endif

namespace engine {

class app {
public:
	explicit app(std::string title): title_{std::move(title)} {}
	virtual ~app() = default;

	// Non-copyable
	app(const app &) = delete;
	auto operator=(const app &) -> app & = delete;

	// Non-movable
	app(app &&) noexcept = delete;
	auto operator=(app &&) noexcept -> app & = delete;

	[[nodiscard]] auto run() -> result<>;

	struct version {
		int major{};
		int minor{};
		int patch{};
		int build{};
	};

	[[nodiscard]] auto get_version() const -> const version & {
		return version_;
	}

	[[nodiscard]] auto get_default_font() const -> const Font & {
		return default_font_;
	}

	[[nodiscard]] auto get_default_font_size() const -> const int & {
		return default_font_size_;
	}

	template<typename Event>
	auto subscribe(std::function<void(const Event &)> handler) -> event_bus::token_t {
		return event_bus_.subscribe<Event>(std::move(handler));
	}

	template<typename Event, typename T, typename Func>
	auto bind_event(T *instance, Func func) -> event_bus::token_t {
		return subscribe<Event>([instance, func](const Event &evt) -> auto { (instance->*func)(evt); });
	}

	template<typename Event, typename T, typename Func>
	auto on_event(T *instance, Func func) -> event_bus::token_t {
		return subscribe<Event>([instance, func](const Event &) -> auto { (instance->*func)(); });
	}

	auto unsubscribe(const event_bus::token_t token) -> void {
		event_bus_.unsubscribe(token);
	}

	template<typename Event>
	auto post_event(const Event &event) -> void {
		event_bus_.post(event);
	}

	[[nodiscard]] auto play_sound(const std::string &name) -> result<>;

protected:
	[[nodiscard]] virtual auto init() -> result<>;
	[[nodiscard]] virtual auto init_scenes() -> result<>;
	[[nodiscard]] virtual auto end() -> result<>;
	[[nodiscard]] virtual auto update() -> result<>;
	[[nodiscard]] virtual auto draw() const -> result<>;

	auto set_clear_color(const Color &color) -> void {
		clear_color_ = color;
	}

	template<typename T>
		requires std::is_base_of_v<scene, T>
	auto register_scene(int layer = 0, const bool visible = true) -> int {
		int scene_id = ++last_scene_id_;

		std::string name;
#if defined(__GNUG__) && !defined(__EMSCRIPTEN__) && !defined(__APPLE__)
		int status = 0;
		const char *mangled = typeid(T).name();
		using demangle_ptr = std::unique_ptr<char, decltype(&std::free)>;
		demangle_ptr const demangled{abi::__cxa_demangle(mangled, nullptr, nullptr, &status), &std::free};
		name = (status == 0 && demangled) ? demangled.get() : mangled;
#else
		name = typeid(T).name();
#endif
		SPDLOG_DEBUG("registering scene of type `{}` with id {} at layer {}", name, scene_id, layer);
		scenes_.push_back(scene_info{
			.id = scene_id, .name = name, .scene_ptr = std::make_unique<T>(), .layer = layer, .visible = visible});
		sort_scenes();
		return scene_id;
	}

	template<typename T>
		requires std::is_base_of_v<scene, T>
	auto register_scene(const bool visible) -> int {
		return register_scene<T>(0, visible);
	}

	auto unregister_scene(const int scene_id) -> result<> {
		const auto find =
			std::ranges::find_if(scenes_, [scene_id](const scene_info &scene) -> bool { return scene.id == scene_id; });
		if(find != scenes_.end()) {
			if(find->scene_ptr) {
				if(const auto err = find->scene_ptr->end().ko(); err) {
					return error(std::format("error ending scene with id: {} name: {}", scene_id, find->name), *err);
				}
				find->scene_ptr.reset();
			}
			scenes_.erase(find);
			return true;
		}
		return error(std::format("scene with id {} not found", scene_id));
	}

	auto sort_scenes() -> void {
		std::ranges::sort(scenes_, [](const scene_info &scene_a, const scene_info &scene_b) -> bool {
			return scene_a.layer < scene_b.layer;
		});
	}

	[[nodiscard]] auto enable_scene(const int scene_id, const bool enabled = true) -> result<> {
		if(auto [scene_info_res, error] = find_scene_info(scene_id).ok(); !error) {
			scene_info_res->get().visible = enabled;
			return true;
		}
		return error(std::format("scene with id {} not found", scene_id));
	}

	[[nodiscard]] auto disable_scene(const int scene_id, const bool disabled = true) -> result<> {
		return enable_scene(scene_id, !disabled);
	}

	[[nodiscard]] auto set_default_font(const std::string &path, int size, int texture_filter = TEXTURE_FILTER_POINT)
		-> result<>;

	[[nodiscard]] auto load_sound(const std::string &name, const std::string &path) -> result<>;
	[[nodiscard]] auto unload_sound(const std::string &name) -> result<>;

	[[nodiscard]] auto play_music(const std::string &path, bool loop = true) -> result<>;
	[[nodiscard]] auto stop_music() -> result<>;

private:
	Font default_font_{};
	int default_font_size_{12};

	bool custom_default_font_{false};
	int last_scene_id_{0};

	std::string title_{"Engine App"};
	Vector2 screen_size_{};
	static constexpr auto version_file_path = "resources/version/version.json";
	version version_{};

	Color clear_color_ = WHITE;

	[[nodiscard]] auto setup_log() -> result<>;
	[[nodiscard]] static auto parse_version(const std::string &path) -> result<version>;
	static void log_callback(int log_level, const char *text, va_list args);

	struct scene_info {
		int id{};
		std::string name;
		std::unique_ptr<scene> scene_ptr{nullptr};
		int layer{};
		bool visible{};
	};

	std::vector<scene_info> scenes_;

	auto find_scene_info(const int scene_id) -> result<std::reference_wrapper<scene_info>> {
		for(auto &scene_info: scenes_) {
			if(scene_info.id == scene_id) {
				return std::ref(scene_info);
			}
		}
		return error(std::format("scene with id {} not found", scene_id));
	}

	auto set_default_font(const Font &font, int size, int texture_filter = TEXTURE_FILTER_POINT) -> void;

	event_bus event_bus_;

	auto init_sound() -> result<>;
	auto end_sound() -> result<>;
	bool sound_initialized_{false};

	std::map<std::string, Sound> sounds_;

	Music background_music_{};
	bool music_playing_{false};

	auto update_music_stream() const -> void;
};

} // namespace engine
