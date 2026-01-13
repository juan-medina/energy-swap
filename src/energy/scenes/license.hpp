// SPDX-FileCopyrightText: 2026 Juan Medina
// SPDX-License-Identifier: MIT

#pragma once

#include <pxe/components/button.hpp>
#include <pxe/components/component.hpp>
#include <pxe/components/scroll_text.hpp>
#include <pxe/result.hpp>
#include <pxe/scenes/scene.hpp>

namespace pxe {
class app;
struct size;
} // namespace pxe

namespace engine {
class app;
struct size;
} // namespace engine

namespace energy {

class license: public pxe::scene {
public:
	license() = default;
	~license() override = default;

	// Copyable
	license(const license &) = default;
	auto operator=(const license &) -> license & = default;

	// Movable
	license(license &&) noexcept = default;
	auto operator=(license &&) noexcept -> license & = default;

	[[nodiscard]] auto init(pxe::app &app) -> pxe::result<> override;
	[[nodiscard]] auto end() -> pxe::result<> override;

	[[nodiscard]] auto update(float delta) -> pxe::result<> override;
	[[nodiscard]] auto draw() -> pxe::result<> override;

	auto layout(pxe::size screen_size) -> pxe::result<> override;

	struct accepted {};

private:
	static constexpr auto license_path = "resources/license/license.txt";
	pxe::scroll_text scroll_text_;
	pxe::button accept_button_;
	int button_click_{0};

	auto on_button_click(const pxe::button::click &evt) -> pxe::result<>;
};

} // namespace energy
