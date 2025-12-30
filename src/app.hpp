// SPDX-FileCopyrightText: 2025 Juan Medina
// SPDX-License-Identifier: MIT

#pragma once

#include "components/version_display.hpp"
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

    [[nodiscard]] auto run() -> result<>;
    [[nodiscard]] auto init() -> result<>;

private:
    Vector2 screen_size_{};
    static constexpr auto version_file_path = "resources/version/version.json";
    version version_{};

    [[nodiscard]] auto update() -> result<>;
    void draw() const;

    [[nodiscard]] auto setup_log() -> result<>;
    [[nodiscard]] static auto parse_version(const std::string &path) -> result<version>;

    static void log_callback(int log_level, const char *text, va_list args);

    // components
    version_display version_display_;
};

} // namespace energy
