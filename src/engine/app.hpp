// SPDX-FileCopyrightText: 2026 Juan Medina
// SPDX-License-Identifier: MIT

#pragma once

#include "result.hpp"
#include "scenes/scene.hpp"

#include <raylib.h>

#include <algorithm>
#include <memory>
#include <spdlog/spdlog.h>
#include <vector>

#if defined(__GNUG__) && !defined(__EMSCRIPTEN__) && !defined(__APPLE__)
#    include <cxxabi.h>
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

protected:
    [[nodiscard]] virtual auto init() -> result<>;
    [[nodiscard]] virtual auto init_scenes() -> result<>;
    [[nodiscard]] virtual auto end() -> result<>;
    [[nodiscard]] virtual auto update() -> result<>;
    [[nodiscard]] virtual auto draw() const -> result<>;

    template<typename T>
        requires std::is_base_of_v<scene, T>
    auto register_scene(int layer = 0, bool visible = true) -> int {
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

    auto unregister_scene(const int scene_id) -> result<> {
        const auto find =
            std::ranges::find_if(scenes_, [scene_id](const scene_info &scene) -> bool { return scene.id == scene_id; });
        if(find != scenes_.end()) {
            if(find->scene_ptr) {
                if(const auto err = find->scene_ptr->end().ko(); err) {
                    return error(std::format("Error ending scene with id: {} name: {}", scene_id, find->name), *err);
                }
                find->scene_ptr.reset();
            }
            scenes_.erase(find);
            return true;
        }
        return error(std::format("Scene with id {} not found", scene_id));
    }

    auto sort_scenes() -> void {
        std::ranges::sort(scenes_, [](const scene_info &scene_a, const scene_info &scene_b) -> bool {
            return scene_a.layer < scene_b.layer;
        });
    }

    auto enable_scene(const int scene_id, bool enabled = true) -> result<> {
        if(auto [scene_info_res, error] = find_scene_info(scene_id).ok(); error) {
            scene_info_res->get().visible = enabled;
            return true;
        }
        return error(std::format("Scene with id {} not found", scene_id));
    }

    [[nodiscard]] auto set_default_font(const std::string &path, int size, int texture_filter = TEXTURE_FILTER_POINT)
        -> result<>;

private:
    Font default_font_{};
    bool custom_default_font_{false};
    int last_scene_id_{0};

    std::string title_{"Engine App"};
    Vector2 screen_size_{};
    static constexpr auto version_file_path = "resources/version/version.json";
    version version_{};

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
        return error(std::format("Scene with id {} not found", scene_id));
    }

    auto set_default_font(const Font &font, int texture_filter = TEXTURE_FILTER_POINT) -> void;
};

} // namespace engine
