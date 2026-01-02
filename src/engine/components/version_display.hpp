// SPDX-FileCopyrightText: 2026 Juan Medina
// SPDX-License-Identifier: MIT

#pragma once

#include "ui_component.hpp"
#include "../scenes/scene.hpp"

#include <raylib.h>

#include <array>

namespace engine {

class version_display: public ui_component {
public:
    version_display() = default;
    ~version_display() override = default;

    // Non-copyable
    version_display(const version_display &) = delete;
    auto operator=(const version_display &) -> version_display & = delete;

    // Non-movable
    version_display(version_display &&) noexcept = delete;
    auto operator=(version_display &&) noexcept -> version_display & = delete;

    [[nodiscard]] auto init(app &app) -> result<> override;
    [[nodiscard]] auto end() -> result<> override;

    [[nodiscard]] auto update(float delta) -> result<> override;
    [[nodiscard]] auto draw() -> result<> override;

    auto set_font_size(const float &size) -> void override;
private:
    struct part {
        std::string text;
        Color color;
        float offset;
    };

    static constexpr std::array components_colors = {
        Color{.r = 0xF0, .g = 0x00, .b = 0xF0, .a = 0xFF}, // v
        Color{.r = 0xFF, .g = 0x00, .b = 0x00, .a = 0xFF}, // major
        Color{.r = 0xFF, .g = 0xFF, .b = 0xFF, .a = 0xFF}, // .
        Color{.r = 0xFF, .g = 0xA5, .b = 0x00, .a = 0xFF}, // minor
        Color{.r = 0xFF, .g = 0xFF, .b = 0xFF, .a = 0xFF}, // .
        Color{.r = 0xFF, .g = 0xFF, .b = 0x00, .a = 0xFF}, // patch
        Color{.r = 0xFF, .g = 0xFF, .b = 0xFF, .a = 0xFF}, // .
        Color{.r = 0x00, .g = 0xFF, .b = 0x00, .a = 0xFF}  // build
    };

    std::array<part, 8> parts_{};

    float parts_spacing_ = 0.0F;
    float shadow_offset_ = 0.0F;

    static auto open_url(const std::string &url) -> result<>;

    auto draw_parts(Vector2 pos, bool shadow) -> void;
    bool hover_{false};

};
} // namespace engine