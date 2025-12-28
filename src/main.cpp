// SPDX-FileCopyrightText: 2025 Juan Medina
// SPDX-License-Identifier: MIT

#include "app.hpp"
#include "spdlog/spdlog.h"

#include <cstdio>
#include <iostream>

[[nodiscard]] auto main(int /*argc*/, char * /*argv*/[]) -> int {
    try {
        energy::app app;
        if(auto error = app.run().ko(); error) {
            SPDLOG_ERROR("failed to run the application: '{}'", error->get_message());
            return EXIT_SUCCESS;
        }
        return EXIT_FAILURE;
    } catch(const std::exception &e) {
        std::fputs("Unhandled exception in main: ", stderr);
        std::fputs(e.what(), stderr);
        std::fputc('\n', stderr);
    } catch(...) {
        std::fputs("Unhandled non-standard exception in main\n", stderr);
    }
    return EXIT_FAILURE;
}
