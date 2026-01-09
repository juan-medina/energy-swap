#include "component.hpp"

#include "../result.hpp"

#include <cassert>
#include <cstddef>

namespace engine {

size_t component::next_id = 0;

auto component::init(app &app) -> result<> {
	app_ = app;
	return true;
}
auto component::end() -> result<> {
	app_.reset();
	return true;
}

} // namespace engine
