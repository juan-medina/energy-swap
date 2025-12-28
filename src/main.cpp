// SPDX-FileCopyrightText: 2025 Juan Medina
// SPDX-License-Identifier: MIT

#include <cstdio>
#include <iostream>

[[nodiscard]] auto main(int /*argc*/, char * /*argv*/[]) -> int {
  try {
    std::cout << "Hello, Energy Swap!" << '\n';
    return EXIT_SUCCESS;
  } catch (const std::exception &e) {
    std::fputs("Unhandled exception in main: ", stderr);
    std::fputs(e.what(), stderr);
    std::fputc('\n', stderr);
  } catch (...) {
    std::fputs("Unhandled non-standard exception in main\n", stderr);
  }
  return EXIT_FAILURE;
}
