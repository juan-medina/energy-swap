// SPDX-FileCopyrightText: 2025 Juan Medina
// SPDX-License-Identifier: MIT

#include <cstdio>
#include <iostream>
#include <spdlog/spdlog.h>

[[nodiscard]] auto main(int /*argc*/, char * /*argv*/[]) -> int {
    try {
        spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] [%@] %v");
        SPDLOG_INFO("Hello from spdlog!");
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
