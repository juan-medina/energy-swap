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

    [[nodiscard]] auto run() -> result<>;
    [[nodiscard]] auto init() -> result<>;

    struct version {
        int major{};
        int minor{};
        int patch{};
        int build{};
    };

private:
    static constexpr auto version_file_path = "resources/version/version.json";
    version version_{};

    static void update();
    void draw() const;

    [[nodiscard]] auto setup_log() -> result<>;
    [[nodiscard]] static auto parse_version(const std::string &path) -> result<version>;

    static void log_callback(int log_level, const char *text, va_list args);

    auto draw_version() const -> void;
};

} // namespace energy
