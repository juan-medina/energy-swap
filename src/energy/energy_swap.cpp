// SPDX-FileCopyrightText: 2025 Juan Medina
// SPDX-License-Identifier: MIT

#include "energy_swap.hpp"

#include "scenes/license.hpp"

#include <spdlog/spdlog.h>

auto energy::energy_swap::init() -> engine::result<> {
    if(const auto err = app::init().ko(); err) {
        return engine::error{"failed to initialize base app", *err};
    }

    register_scene<license>();

    return true;
}
