// SPDX-FileCopyrightText: 2025 Juan Medina
// SPDX-License-Identifier: MIT

#include "app.hpp"

#include <boxer/boxer.h>
#include <cstdio>

[[nodiscard]] auto main(int /*argc*/, char * /*argv*/[]) -> int {
    try {
        energy::app app;
        if(const auto error = app.run().ko(); error) {
            boxer::show("Failed to run the application", "Error!", boxer::Style::Error);
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
