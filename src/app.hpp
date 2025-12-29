// SPDX-FileCopyrightText: 2025 Juan Medina
// SPDX-License-Identifier: MIT

#pragma once

#include "result.hpp"

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

    [[nodiscard]] static auto run() -> result<>;

private:
    static void update();
    static void draw();
    static void setup_log();
    static void log_callback(int log_level, const char *text, va_list args);
};

} // namespace energy
