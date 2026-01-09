#include "scene.hpp"

#include "../components/component.hpp"
#include "../result.hpp"

#include <algorithm>
#include <format>

namespace engine {

auto scene::end() -> result<> {
	for(auto &child: children_) {
		if(const auto err = child.comp->end().unwrap(); err) {
			return error(std::format("error ending component with id: {}", child.id), *err);
		}
	}
	return component::end();
}
auto scene::update(const float delta) -> result<> {
	for(auto &child: children_) {
		if(const auto err = child.comp->update(delta).unwrap(); err) {
			return error(std::format("error updating component with id: {}", child.id), *err);
		}
	}
	return component::update(delta);
}

auto scene::draw() -> result<> {
	std::ranges::sort(children_, [](const child &a, const child &b) -> bool { return a.layer < b.layer; });
	for(auto &child: children_) {
		if(const auto err = child.comp->draw().unwrap(); err) {
			return error(std::format("error drawing component with id: {}", child.id), *err);
		}
	}
	return component::draw();
}

} // namespace engine