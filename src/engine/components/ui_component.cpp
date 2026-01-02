// SPDX-FileCopyrightText: 2026 Juan Medina
// SPDX-License-Identifier: MIT

#include "ui_component.hpp"

#include "../app.hpp"

namespace engine {

auto ui_component::init(app &app) -> result<> {
    set_font(app.get_default_font());
    set_font_size(static_cast<float>(app.get_default_font_size()));

    return true;
}

auto ui_component::end() -> result<> {
    return true;
}

auto ui_component::update(float /*delta*/) -> result<> {
    return true;
}

auto ui_component::draw() -> result<> {
    return true;
}

} // namespace engine