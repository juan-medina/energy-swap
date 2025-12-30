// SPDX-FileCopyrightText: 2025 Juan Medina
// SPDX-License-Identifier: MIT

#pragma once

#include "../result.hpp"

#include <raylib.h>

namespace energy {
class component {
public:
    component() = default;
    virtual ~component() = default;

    // Non-copyable
    component(const component &) = delete;
    auto operator=(const component &) -> component & = delete;

    // Non-movable
    component(component &&) noexcept = delete;
    auto operator=(component &&) noexcept -> component & = delete;

    virtual auto layout(Vector2 screen_size) -> void = 0;
    [[nodiscard]] virtual auto update(float delta) -> result<> = 0;
    virtual auto draw() const -> void = 0;
};
} // namespace energy