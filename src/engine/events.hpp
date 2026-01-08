// SPDX-FileCopyrightText: 2026 Juan Medina
// SPDX-License-Identifier: MIT

#pragma once

#include "result.hpp"

#include <algorithm>
#include <functional>
#include <map>
#include <memory>
#include <queue>
#include <typeindex>
#include <utility>
#include <vector>

namespace engine {

class event_bus {
public:
	event_bus() = default;
	~event_bus() = default;

	// Non-copyable / non-movable
	event_bus(const event_bus &) = delete;
	auto operator=(const event_bus &) -> event_bus & = delete;
	event_bus(event_bus &&) noexcept = delete;
	auto operator=(event_bus &&) noexcept -> event_bus & = delete;

	template<typename Event>
	auto subscribe(std::function<result<>(const Event &)> handler) -> int {
		const auto key = std::type_index(typeid(Event));
		const int token_id = ++last_token_;

		auto wrapper = [wrapp_handler = std::move(handler)](const void *evt_ptr) -> auto {
			return wrapp_handler(*static_cast<const Event *>(evt_ptr));
		};

		subscribers_[key].emplace_back(subscriber{.id = token_id, .func = std::move(wrapper)});
		return token_id;
	}

	auto unsubscribe(const int token) -> void {
		for(auto it = subscribers_.begin(); it != subscribers_.end();) {
			auto &vec = it->second;
			std::erase_if(vec, [token](const subscriber &sub) -> bool { return sub.id == token; });
			if(vec.empty()) {
				it = subscribers_.erase(it);
			} else {
				++it;
			}
		}
	}

	template<typename Event>
	auto post(const Event &event) -> void {
		// Clangd has trouble with std::static_pointer_cast in this context
		// however it does not error on compilation, nor on runtime
		// so we just do a empty statement just for the editor
#ifdef CLANGD_ACTIVE
		(void)event; // avoid unused variable warning
#else
		auto blob = std::make_shared<Event>(event);
		queued_.push(queued_item{std::type_index(typeid(Event)), std::static_pointer_cast<void>(blob)});
#endif
	}

	[[nodiscard]] auto dispatch() -> result<> {
		std::queue<queued_item> local_queue;
		{
			if(queued_.empty()) {
				return true;
			}
			std::swap(local_queue, queued_);
		}

		while(!local_queue.empty()) {
			const auto &[type, payload] = local_queue.front();
			if(const auto err = dispatch_erased(type, payload.get()).ko(); err) {
				return error("dispatch erased failed", *err);
			}
			local_queue.pop();
		}
		return true;
	}

private:
	struct subscriber {
		int id{};
		std::function<result<>(const void *)> func;
	};

	struct queued_item {
		std::type_index type;
		std::shared_ptr<void> payload;
	};

	std::map<std::type_index, std::vector<subscriber>> subscribers_;
	std::queue<queued_item> queued_;
	int last_token_{0};

	[[nodiscard]] auto dispatch_erased(const std::type_index &type, const void *payload) -> result<> {
		std::vector<std::function<result<>(const void *)>> handlers;
		{
			const auto find = subscribers_.find(type);
			if(find == subscribers_.end()) {
				return true;
			}
			handlers.reserve(find->second.size());
			for(const auto &[event_id, func]: find->second) {
				handlers.push_back(func);
			}
		}
		for(const auto &func: handlers) {
			if(const auto err = func(payload).ko(); err) {
				return error("event handler function failed", *err);
			}
		}
		return true;
	}
};

} // namespace engine