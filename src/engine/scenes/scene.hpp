// SPDX-FileCopyrightText: 2026 Juan Medina
// SPDX-License-Identifier: MIT

#pragma once

#include "../components/component.hpp"
#include "../result.hpp"
#include "spdlog/spdlog.h"

#include <algorithm>
#include <format>
#include <functional>
#include <memory>
#include <optional>
#include <ranges>
#include <typeinfo>
#include <utility>
#include <vector>

namespace engine {

class app;

class scene: public component {
public:
	scene() = default;
	~scene() override = default;

	// Non-copyable
	scene(const scene &) = delete;
	auto operator=(const scene &) -> scene & = delete;

	// Non-movable
	scene(scene &&) noexcept = delete;
	auto operator=(scene &&) noexcept -> scene & = delete;

	[[nodiscard]] auto init(app &app) -> result<> override {
		return component::init(app);
	}

	[[nodiscard]] auto end() -> result<> override;

	[[nodiscard]] virtual auto enable() -> result<> {
		return true;
	}

	[[nodiscard]] virtual auto disable() -> result<> {
		return true;
	}

	[[nodiscard]] auto update(float delta) -> result<> override;

	[[nodiscard]] auto draw() -> result<> override;

	[[nodiscard]] virtual auto layout(const size /*screen_size*/) -> result<> {
		return true;
	}

	template<typename T, typename... Args>
		requires std::is_base_of_v<component, T>
	[[nodiscard]] auto register_component(Args &&...args) -> result<int> {
		auto comp = std::make_shared<T>();
		if(const auto err = comp->init(get_app(), std::forward<Args>(args)...).ko(); err) {
			return error(std::format("error initializing component of type: {}", typeid(T).name()), *err);
		}
		children_.emplace_back(child{.id = ++last_child_id_, .comp = std::move(comp), .layer = 0});
		SPDLOG_DEBUG("component of type `{}` registered with id {}", typeid(T).name(), last_child_id_);
		return last_child_id_;
	}

	[[nodiscard]] auto remove_component(const int id) -> result<> {
		const auto it = std::ranges::find_if(children_, [id](const child &c) -> bool { return c.id == id; });
		if(it == children_.end()) {
			return error(std::format("no component found with id: {}", id));
		}
		if(const auto err = it->comp->end().ko(); err) {
			return error(std::format("error ending component with id: {}", id), *err);
		}
		children_.erase(it);
		SPDLOG_DEBUG("component with id {} removed", id);
		return true;
	}

	template<typename T>
		requires std::is_base_of_v<component, T>
	[[nodiscard]] auto get_component(const int id) -> result<std::shared_ptr<T>> {
		const auto it = std::ranges::find_if(children_, [id](const child &c) -> auto { return c.id == id; });
		if(it == children_.end()) {
			return error(std::format("no component found with id: {}", id));
		}
		auto comp = std::dynamic_pointer_cast<T>(it->comp);
		if(!comp) {
			return error(std::format("component with id: {} is not of type: {}", id, typeid(T).name()));
		}
		return comp;
	}

private:
	int last_child_id_{0};
	struct child {
		int id = 0;
		std::shared_ptr<component> comp;
		int layer = 0;
	};
	std::vector<child> children_;
};
} // namespace engine