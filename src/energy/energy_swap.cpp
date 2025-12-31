// SPDX-FileCopyrightText: 2025 Juan Medina
// SPDX-License-Identifier: MIT

#include "energy_swap.hpp"
auto energy::energy_swap::init() -> engine::result<> {
    return app::init();
}
auto energy::energy_swap::update() -> engine::result<> {
    return app::update();
}
auto energy::energy_swap::draw() const -> engine::result<> {
    return app::draw();
}