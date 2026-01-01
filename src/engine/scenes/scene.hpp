// SPDX-FileCopyrightText: 2025 Juan Medina
// SPDX-License-Identifier: MIT

#pragma once

#include "../components/component.hpp"
#include "../result.hpp"

#include <raylib.h>

namespace engine {

class app;

class scene : public component {
public:
    scene() = default;
    ~scene() override = default;

    // Non-copyable
    scene(const scene &) = delete;
    auto operator=(const scene &) -> scene & = delete;

    // Non-movable
    scene(scene &&) noexcept = delete;
    auto operator=(scene &&) noexcept -> scene & = delete;

    [[nodiscard]] auto init(app &app) -> result<> override = 0;
    [[nodiscard]] auto end() -> result<> override = 0;

    [[nodiscard]] auto update(float delta) -> result<> override = 0;
    [[nodiscard]] auto draw() -> result<> override = 0;

    virtual auto layout(Vector2 screen_size) -> void = 0;
};
} // namespace engine