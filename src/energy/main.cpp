// SPDX-FileCopyrightText: 2026 Juan Medina
// SPDX-License-Identifier: MIT

#include "energy_swap.hpp"

#ifndef __EMSCRIPTEN__
#	include <boxer/boxer.h>
#endif

#include <cstdio>
#include <spdlog/spdlog.h>

[[nodiscard]] auto main(int /*argc*/, char * /*argv*/[]) -> int {
	try {
		energy::energy_swap app;
		if(const auto error = app.run().ko(); error) {
			SPDLOG_ERROR("{}", error->to_string());
#ifndef __EMSCRIPTEN__
			boxer::show(error->get_message().c_str(), "Error!", boxer::Style::Error);
#endif
			return EXIT_FAILURE;
		}
		return EXIT_SUCCESS;
	} catch(const std::exception &e) {
		std::fputs("Unhandled exception in main: ", stderr);
		std::fputs(e.what(), stderr);
		std::fputc('\n', stderr);
	} catch(...) {
		std::fputs("Unhandled non-standard exception in main\n", stderr);
	}
	return EXIT_FAILURE;
}
