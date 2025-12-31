// SPDX-FileCopyrightText: 2025 Juan Medina
// SPDX-License-Identifier: MIT

#pragma once

#include "component.hpp"

#include <array>
#include <raylib.h>

namespace engine {

class version_display: public component {
public:
    version_display() = default;
    ~version_display() override = default;

    // Non-copyable
    version_display(const version_display &) = delete;
    auto operator=(const version_display &) -> version_display & = delete;

    // Non-movable
    version_display(version_display &&) noexcept = delete;
    auto operator=(version_display &&) noexcept -> version_display & = delete;

    [[nodiscard]] auto init(app *app) -> result<> override;
    [[nodiscard]] auto end() -> result<> override;

    auto layout(Vector2 screen_size) -> void override;
    [[nodiscard]] auto update(float delta) -> result<> override;
    [[nodiscard]] auto draw() const -> result<> override;

private:
    struct part {
        std::string text;
        Color color;
        float offset;
    };

    Rectangle bounds_{};
    Vector2 screen_size_{};
    Font font_{};

    static constexpr std::array components_colors = {
        Color{.r = 0xF0, .g = 0x00, .b = 0xF0, .a = 0xFF}, // #F000F0 (v)
        Color{.r = 0xFF, .g = 0x00, .b = 0x00, .a = 0xFF}, // #FF0000 (major)
        Color{.r = 0xFF, .g = 0xFF, .b = 0xFF, .a = 0xFF}, // #F000F0 (.)
        Color{.r = 0xFF, .g = 0xA5, .b = 0x00, .a = 0xFF}, // #FFA500 (minor)
        Color{.r = 0xFF, .g = 0xFF, .b = 0xFF, .a = 0xFF}, // #F000F0 (.)
        Color{.r = 0xFF, .g = 0xFF, .b = 0x00, .a = 0xFF}, // #FFFF00 (patch)
        Color{.r = 0xFF, .g = 0xFF, .b = 0xFF, .a = 0xFF}, // #F000F0 (.)
        Color{.r = 0x00, .g = 0xFF, .b = 0x00, .a = 0xFF}  // #00FF00 (build)
    };

    std::array<part, 8> parts_{};

    static constexpr auto font_size = 20.0F;
    static constexpr auto parts_spacing = 4.0F;
    static constexpr auto margin = 10.0F;

    static auto open_url(const std::string &url) -> result<>;
};
} // namespace engine