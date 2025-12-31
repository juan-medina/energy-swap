// SPDX-FileCopyrightText: 2025 Juan Medina
// SPDX-License-Identifier: MIT

#pragma once

#include "../engine/app.hpp"

namespace energy {

class energy_swap: public engine::app {
public:
    energy_swap(): app("Energy Swapper") {}
    ~energy_swap() override = default;

    // Non-copyable
    energy_swap(const energy_swap &) = delete;
    auto operator=(const energy_swap &) -> energy_swap & = delete;

    // Non-movable
    energy_swap(energy_swap &&) noexcept = delete;
    auto operator=(energy_swap &&) noexcept -> energy_swap & = delete;

protected:
    [[nodiscard]] auto init() -> engine::result<> override;
    [[nodiscard]] auto update() -> engine::result<> override;
    [[nodiscard]] auto draw() const -> engine::result<> override;
};

} // namespace energy