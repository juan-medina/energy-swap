// SPDX-FileCopyrightText: 2026 Juan Medina
// SPDX-License-Identifier: MIT

#pragma once

#include "component.hpp"

namespace engine {

class ui_component: public component {
public:
    ui_component() = default;
    ~ui_component() override = default;

    // Non-copyable
    ui_component(const ui_component &) = delete;
    auto operator=(const ui_component &) -> ui_component & = delete;

    // Non-movable
    ui_component(ui_component &&) noexcept = delete;
    auto operator=(ui_component &&) noexcept -> ui_component & = delete;

    [[nodiscard]] auto init(app &app) -> result<> override;
    [[nodiscard]] auto end() -> result<> override;

    [[nodiscard]] auto update(float delta) -> result<> override;
    [[nodiscard]] auto draw() -> result<> override;

    auto set_font(const Font &font) -> void {
        font_ = font;
    }

    [[nodiscard]] auto get_font() const -> Font {
        return font_;
    }

    virtual auto set_font_size(const float &size) -> void {
        font_size_ = size;
    }

    [[nodiscard]] auto get_font_size() const -> float {
        return font_size_;
    }

    auto set_click_sound(const std::string &sound_name) -> void {
        click_sound_ = sound_name;
    }

    [[nodiscard]] auto play_click_sound() -> result<>;

private:
    Font font_{};
    float font_size_ = 20.0F;
    std::string click_sound_{"click"};
};

} // namespace engine
