// SPDX-FileCopyrightText: 2025 Juan Medina
// SPDX-License-Identifier: MIT

#pragma once

#include "result.hpp"
#include "scenes/scene.hpp"
#include "spdlog/spdlog.h"

#include <algorithm>
#include <memory>
#include <vector>

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

protected:
    [[nodiscard]] virtual auto init() -> result<>;
    [[nodiscard]] virtual auto init_scenes() -> result<>;
    [[nodiscard]] virtual auto end() -> result<>;
    [[nodiscard]] virtual auto update() -> result<>;
    [[nodiscard]] virtual auto draw() const -> result<>;

    auto register_scene(const int scene_id, std::unique_ptr<scene> scene, const int layer, const bool visible) -> void {
        SPDLOG_DEBUG("Registering scene with id {} at layer {}", scene_id, layer);
        scenes_.push_back(
            scene_info{.id = scene_id, .scene_ptr = std::move(scene), .layer = layer, .visible = visible});
        sort_scenes();
    }

    auto unregister_scene(const int scene_id) -> result<> {
        const auto find =
            std::ranges::find_if(scenes_, [scene_id](const scene_info &scene) -> bool { return scene.id == scene_id; });
        if(find != scenes_.end()) {
            if(find->scene_ptr) {
                if(const auto err = find->scene_ptr->end().ko(); err) {
                    return error(std::format("Error ending scene with id {}", scene_id), *err);
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

private:
    std::string title_{"Engine App"};
    Vector2 screen_size_{};
    static constexpr auto version_file_path = "resources/version/version.json";
    version version_{};

    [[nodiscard]] auto setup_log() -> result<>;
    [[nodiscard]] static auto parse_version(const std::string &path) -> result<version>;
    static void log_callback(int log_level, const char *text, va_list args);

    struct scene_info {
        int id{};
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
};

} // namespace engine
