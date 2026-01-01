// SPDX-FileCopyrightText: 2025 Juan Medina
// SPDX-License-Identifier: MIT

#include "scroll_text.hpp"

#include "raygui.h"
#include "spdlog/spdlog.h"

#include <algorithm>
#include <sstream>

namespace engine {

auto scroll_text::init(app & /*app*/) -> result<> {
    font_ = GetFontDefault();
    return true;
}

auto scroll_text::end() -> result<> {
    return true;
}

auto scroll_text::update(float /*delta*/) -> result<> {
    return true;
}

auto scroll_text::draw() -> result<> {
    GuiSetStyle(DEFAULT, TEXT_SIZE, static_cast<int>(font_size_));

    auto [x, y] = get_pos();
    const auto [width, height] = get_size();
    const auto bound = Rectangle{.x = x, .y = y, .width = width, .height = height};
    GuiScrollPanel(bound, title_.c_str(), content_, &scroll_, &view_);

    BeginScissorMode(static_cast<int>(view_.x),
                     static_cast<int>(view_.y),
                     static_cast<int>(view_.width),
                     static_cast<int>(view_.height));

    auto start_y = view_.y + scroll_.y;
    auto const start_x = view_.x + scroll_.x;

    for(const auto &line: text_lines_) {
        DrawTextEx(font_, line.c_str(), {.x = start_x, .y = start_y}, font_size_, spacing_, BLACK);
        const auto [_, line_y] = MeasureTextEx(font_, line.c_str(), font_size_, spacing_);
        start_y += line_y + line_spacing_;
    }

    EndScissorMode();

    return true;
}

auto scroll_text::set_text(const std::string &text) -> void {
    float max_x = 0;
    float total_height = 0;

    std::istringstream stream(text);
    std::string line;
    while(std::getline(stream, line)) {
        text_lines_.emplace_back(line);

        const auto [x, y] = MeasureTextEx(font_, line.c_str(), font_size_, spacing_);
        max_x = std::max(x, max_x);
        total_height += y + line_spacing_;
    }

    content_.x = 0;
    content_.y = 0;
    content_.width = max_x;
    content_.height = total_height;
    scroll_ = {.x = 0, .y = 0};
    view_ = {.x = 0, .y = 0, .width = 0, .height = 0};
}

} // namespace engine