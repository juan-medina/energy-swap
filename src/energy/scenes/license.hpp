// SPDX-FileCopyrightText: 2026 Juan Medina
// SPDX-License-Identifier: MIT

#pragma once

#include "../../engine/app.hpp"
#include "../../engine/components/button.hpp"
#include "../../engine/components/scroll_text.hpp"
#include "../../engine/events.hpp"
#include "../../engine/result.hpp"
#include "../../engine/scenes/scene.hpp"

namespace energy {

class license: public engine::scene {
public:
    license() = default;
    ~license() override = default;

    // Non-copyable
    license(const license &) = delete;
    auto operator=(const license &) -> license & = delete;

    // Non-movable
    license(license &&) noexcept = delete;
    auto operator=(license &&) noexcept -> license & = delete;

    [[nodiscard]] auto init(engine::app &app) -> engine::result<> override;
    [[nodiscard]] auto end() -> engine::result<> override;

    [[nodiscard]] auto update(float delta) -> engine::result<> override;
    [[nodiscard]] auto draw() -> engine::result<> override;

    auto layout(Vector2 screen_size) -> void override;

    struct accepted {};

private:
    static constexpr auto license_path = "resources/license/license.txt";
    engine::scroll_text scroll_text_;
    engine::button accept_button_;
    engine::event_bus::token_t button_click_{0};

    auto on_button_click(const engine::button::click &evt) -> void;
};

} // namespace energy
