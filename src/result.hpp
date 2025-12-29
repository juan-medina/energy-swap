// SPDX-FileCopyrightText: 2025 Juan Medina
// SPDX-License-Identifier: MIT

#pragma once

#include <format>
#include <optional>
#include <source_location>
#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace energy {

class error {
public:
    explicit error(const std::string &message, const std::source_location &location = std::source_location::current())
        : message_{format_with_location(message, location)} {}

    error(const std::string &message,
          const error &other,
          const std::source_location &location = std::source_location::current())
        : message_{format_with_location(message, location)}, causes_{other.causes_} {
        causes_.insert(causes_.begin(), other.message_);
    }

    [[nodiscard]] auto get_message() const noexcept -> const std::string & {
        return message_;
    }

    [[nodiscard]] auto get_causes() const noexcept -> const std::vector<std::string> & {
        return causes_;
    }

    [[nodiscard]] auto to_string() const -> std::string {
        std::string out = message_;
        for(const auto &cause : causes_) {
            out += "\n  caused by: " + cause;
        }
        return out;
    }

private:
    static auto format_with_location(const std::string &msg, const std::source_location &loc) -> std::string {
        return std::format("{} [{}:{} {}]", msg, loc.file_name(), loc.line(), loc.function_name());
    }

    std::string message_;
    std::vector<std::string> causes_;
};

template<class Value = bool, class Error = error>
class result: public std::variant<Value, Error> {
public:
    // NOLINTNEXTLINE(google-explicit-constructor)
    result(const Value &value): std::variant<Value, Error>(value) {}

    // NOLINTNEXTLINE(google-explicit-constructor)
    result(const Error &error): std::variant<Value, Error>(error) {}

    [[maybe_unused]] auto has_error() const noexcept {
        return std::holds_alternative<Error>(*this);
    }

    [[maybe_unused]] auto has_value() const noexcept {
        return std::holds_alternative<Value>(*this);
    }

    [[maybe_unused]] [[nodiscard]] auto get_error() const {
        return std::get<Error>(*this);
    }

    [[maybe_unused]] [[nodiscard]] auto get_value() const {
        return std::get<Value>(*this);
    }

    [[nodiscard]] auto ok() const noexcept -> std::tuple<std::optional<Value>, std::optional<Error>> {
        if(has_error()) {
            return {std::nullopt, *std::get_if<Error>(this)};
        }
        return {*std::get_if<Value>(this), std::nullopt};
    };

    [[nodiscard]] auto ko() const noexcept -> std::optional<Error> {
        if(has_error()) {
            return *std::get_if<Error>(this);
        }
        return std::nullopt;
    };
};

} // namespace energy