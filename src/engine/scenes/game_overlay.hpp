// SPDX-FileCopyrightText: 2026 Juan Medina
// SPDX-License-Identifier: MIT

#pragma once
#include "../components/version_display.hpp"
#include "../events.hpp"
#include "scene.hpp"

namespace engine {

class game_overlay: public scene {
public:
    game_overlay() = default;
    ~game_overlay() override = default;

    // Non-copyable
    game_overlay(const game_overlay &) = delete;
    auto operator=(const game_overlay &) -> game_overlay & = delete;

    // Non-movable
    game_overlay(game_overlay &&) noexcept = delete;
    auto operator=(game_overlay &&) noexcept -> game_overlay & = delete;

    [[nodiscard]] auto init(app &app) -> result<> override;
    [[nodiscard]] auto end() -> result<> override;

    [[nodiscard]] auto update(float delta) -> result<> override;
    [[nodiscard]] auto draw() -> result<> override;

    auto layout(Vector2 screen_size) -> void override;

private:
    std::optional<std::reference_wrapper<app>> app_;
    version_display version_display_;
    static constexpr auto margin = 10.0F;
    event_bus::token_t click_{0};
    static auto on_version_click() -> void;

    static auto open_url(const std::string &url) -> result<>;
};

} // namespace engine