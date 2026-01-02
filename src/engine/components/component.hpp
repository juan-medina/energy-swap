// SPDX-FileCopyrightText: 2026 Juan Medina
// SPDX-License-Identifier: MIT

#pragma once

#include "../result.hpp"
#include "raylib.h"

namespace engine {

class app;

struct size {
    float width{};
    float height{};
};

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

    [[nodiscard]] virtual auto init(app &app) -> result<> = 0;
    [[nodiscard]] virtual auto end() -> result<> = 0;

    [[nodiscard]] virtual auto update(float delta) -> result<> = 0;
    [[nodiscard]] virtual auto draw() -> result<> = 0;

    auto set_position(const Vector2 &pos) -> void {
        pos_ = pos;
    }

    [[nodiscard]] auto get_pos() const -> const Vector2 & {
        return pos_;
    }

    auto set_size(const size &size) -> void {
        size_ = size;
    }

    [[nodiscard]] auto get_size() const -> const size & {
        return size_;
    }

    static auto point_inside(const Vector2 pos, const size size, const Vector2 point) -> bool {
        return point.x >= pos.x && point.x <= pos.x + size.width && point.y >= pos.y && point.y <= pos.y + size.height;
    }

    [[nodiscard]] auto point_inside(const Vector2 point) const -> bool {
        return point_inside(pos_, size_, point);
    }

private:
    Vector2 pos_{};
    size size_{};
};
} // namespace engine