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

    struct version {
        int major{};
        int minor{};
        int patch{};
        int build{};
    };

private:
    static void update();
    static void draw();

    [[nodiscard]] static auto setup_log() -> result<>;
    [[nodiscard]] static auto parse_version(const std::string &path) -> result<version>;

    static void log_callback(int log_level, const char *text, va_list args);
};

} // namespace energy
