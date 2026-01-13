// SPDX-FileCopyrightText: 2026 Juan Medina
// SPDX-License-Identifier: MIT

#ifdef _WIN32
#	include <windows.h>

#	include <stdlib.h>

extern int main(int argc, char **argv);

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	return main(__argc, __argv);
}
#endif
