// SPDX-FileCopyrightText: 2025 Juan Medina
// SPDX-License-Identifier: MIT

#pragma once

# include "result.hpp"

namespace energy {

class app {
public:
    app() = default;
    ~app() = default;

    // Non-copyable
    app(const app &) = delete;
    auto operator=(const app &) -> app & = delete;

    // Non-movable
    app(app &&) noexcept = delete;
    auto operator=(app &&) noexcept -> app & = delete;

    [[nodiscard]] auto run() -> result<>;

private:
    void update();
    void draw();
};

} // namespace energy
