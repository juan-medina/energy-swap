// SPDX-FileCopyrightText: 2026 Juan Medina
// SPDX-License-Identifier: MIT

#pragma once

#include "component.hpp"

#include <string>
#include <vector>

namespace engine {

class scroll_text: public component {
public:
    scroll_text() = default;
    ~scroll_text() override = default;

    // Non-copyable
    scroll_text(const scroll_text &) = delete;
    auto operator=(const scroll_text &) -> scroll_text & = delete;

    // Non-movable
    scroll_text(scroll_text &&) noexcept = delete;
    auto operator=(scroll_text &&) noexcept -> scroll_text & = delete;

    [[nodiscard]] auto init(app &app) -> result<> override;
    [[nodiscard]] auto end() -> result<> override;

    [[nodiscard]] auto update(float delta) -> result<> override;
    [[nodiscard]] auto draw() -> result<> override;

    auto set_text(const std::string &text) -> void;

    auto set_title(const std::string &title) -> void {
        this->title_ = title;
    }

private:
    auto set_font(const Font &font) -> void {
        font_ = font;
    }

    auto set_font_size(const float font_size) -> void {
        font_size_ = font_size;
        line_spacing_ = font_size_ * 0.5F;
        spacing_ = font_size_ * 0.2F;
    }
    std::string title_;
    std::vector<std::string> text_lines_;

    Vector2 scroll_ = {.x = 0, .y = 0};
    Rectangle view_ = {.x = 0, .y = 0, .width = 0, .height = 0};
    Rectangle content_ = {.x = 0, .y = 0, .width = 0, .height = 0};

    Font font_{};
    float font_size_ = 20;
    float line_spacing_ = font_size_ * 0.5F;
    float spacing_ = font_size_ * 0.2F;
};

} // namespace engine
