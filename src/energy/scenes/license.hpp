// SPDX-FileCopyrightText: 2025 Juan Medina
// SPDX-License-Identifier: MIT

#pragma once

#include "../../engine/result.hpp"
#include "../../engine/scenes/scene.hpp"

namespace energy {

class license: public engine::scene {
public:
    license() = default;
    ~license() override = default;

    // Non-copyable
    license(const license &) = delete;
    auto operator=(const license &) -> license & = delete;

    // Non-movable
    license(license &&) noexcept = delete;
    auto operator=(license &&) noexcept -> license & = delete;

    [[nodiscard]] auto init(engine::app &app) -> engine::result<> override;
    [[nodiscard]] auto end() -> engine::result<> override;

    [[nodiscard]] auto update(float delta) -> engine::result<> override;
    [[nodiscard]] auto draw() const -> engine::result<> override;

    auto layout(Vector2 screen_size) -> void override;
};

} // namespace energy
