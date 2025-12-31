// SPDX-FileCopyrightText: 2025 Juan Medina
// SPDX-License-Identifier: MIT

#pragma once

#include "components/version_display.hpp"
#include "result.hpp"

namespace engine {

class app {
public:
    explicit app(std::string title): title_{std::move(title)} {}
    virtual ~app() = default;

    // Non-copyable
    app(const app &) = delete;
    auto operator=(const app &) -> app & = delete;

    // Non-movable
    app(app &&) noexcept = delete;
    auto operator=(app &&) noexcept -> app & = delete;

    [[nodiscard]] auto run() -> result<>;

    struct version {
        int major{};
        int minor{};
        int patch{};
        int build{};
    };

    [[nodiscard]] auto get_version() const -> const version & {
        return version_;
    }

protected:
    [[nodiscard]] virtual auto init() -> result<>;
    [[nodiscard]] virtual auto update() -> result<>;
    [[nodiscard]] virtual auto draw() const -> result<>;

private:
    std::string title_{"Engine App"};
    Vector2 screen_size_{};
    static constexpr auto version_file_path = "resources/version/version.json";
    version version_{};

    [[nodiscard]] auto setup_log() -> result<>;
    [[nodiscard]] static auto parse_version(const std::string &path) -> result<version>;
    static void log_callback(int log_level, const char *text, va_list args);
    [[nodiscard]] auto internal_draw() const -> result<>;

    // components
    version_display version_display_;
};

} // namespace engine
