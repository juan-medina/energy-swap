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
        : causes_{cause{message, location}} {}

    error(const std::string &message,
          const error &other,
          const std::source_location &location = std::source_location::current())
        : causes_{cause{message, location}} {
        causes_.insert(causes_.end(), other.causes_.begin(), other.causes_.end());
    }

    [[nodiscard]] auto get_message() const noexcept -> const std::string & {
        return causes_.front().get_message();
    }

    [[nodiscard]] auto get_location() const noexcept -> const std::source_location & {
        return causes_.front().get_location();
    }

    [[nodiscard]] auto to_string() const -> std::string {
        std::string out;
        for(size_t i = 0; i < causes_.size(); ++i) {
            const auto &cause = causes_.at(i);
            if(i == 0) {
                out += format_message_with_location(cause.get_message(), cause.get_location());
            } else {
                out += "\n  caused by: " + format_message_with_location(cause.get_message(), cause.get_location());
            }
        }
        return out;
    }

private:
    struct cause {
    private:
        std::string message_;
        std::source_location location_;

    public:
        cause(std::string msg, const std::source_location &loc): message_{std::move(msg)}, location_{loc} {}

        [[nodiscard]] auto get_message() const noexcept -> const std::string & {
            return message_;
        }
        [[nodiscard]] auto get_location() const noexcept -> const std::source_location & {
            return location_;
        }
    };

    static auto format_message_with_location(const std::string &msg, const std::source_location &loc) -> std::string {
        return std::format("{} [{}:{} {}]", msg, loc.file_name(), loc.line(), loc.function_name());
    }

    std::vector<cause> causes_;
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