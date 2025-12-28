// SPDX-FileCopyrightText: 2025 Juan Medina
// SPDX-License-Identifier: MIT

#pragma once

#include <string>
#include <utility>
#include <vector>
#include <optional>
#include <variant>

namespace energy {

class error {
public:
    explicit error(std::string message): message_{std::move(message)} {}
    error(std::string message, const error &other): message_{std::move(message)}, causes_{other.causes_} {
        causes_.insert(causes_.begin(), other.message_);
    };

    [[nodiscard]] inline auto get_message() const noexcept -> const std::string & {
        return message_;
    }

    [[nodiscard]] inline auto get_causes() const noexcept -> const std::vector<std::string> & {
        return causes_;
    }

private:
    std::string message_;
    std::vector<std::string> causes_{};
};

template<class Value = bool, class Error = class error>
class result: public std::variant<Value, Error> {
public:
    // cppcheck-suppress noExplicitConstructor
    // NOLINTNEXTLINE(google-explicit-constructor)
    inline result(const Value &value): std::variant<Value, Error>(value) {}

    // cppcheck-suppress noExplicitConstructor
    // NOLINTNEXTLINE(google-explicit-constructor)
    inline result(const Error &error): std::variant<Value, Error>(error) {}

    [[maybe_unused]] inline auto has_error() const noexcept {
        return std::holds_alternative<Error>(*this);
    }

    [[maybe_unused]] inline auto has_value() const noexcept {
        return std::holds_alternative<Value>(*this);
    }

    [[maybe_unused]] [[nodiscard]] inline auto get_error() const {
        return std::get<Error>(*this);
    }


    [[maybe_unused]] [[nodiscard]] inline auto get_value() const {
        return std::get<Value>(*this);
    }

    [[nodiscard]] inline auto ok() const noexcept -> std::tuple<std::optional<Value>, std::optional<Error>> {
        if(has_error()) {
            return {std::nullopt, *std::get_if<Error>(this)};
        }
        return {*std::get_if<Value>(this), std::nullopt};
    };

    [[nodiscard]] inline auto ko() const noexcept -> std::optional<Error> {
        if(has_error()) {
            return *std::get_if<Error>(this);
        }
        return std::nullopt;
    };
};

} // namespace energy