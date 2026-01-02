// SPDX-FileCopyrightText: 2026 Juan Medina
// SPDX-License-Identifier: MIT

#include "energy_swap.hpp"

#include "scenes/license.hpp"

#include <spdlog/spdlog.h>

auto energy::energy_swap::init() -> engine::result<> {
    if(const auto err = app::init().ko(); err) {
        return engine::error{"failed to initialize base app", *err};
    }

    if(const auto err = set_default_font(font_path, font_size).ko(); err) {
        return engine::error{"failed to set default font", *err};
    }

    register_scene<license>();

    return true;
}
