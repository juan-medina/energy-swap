# Energy Swap

[![License: MIT](https://img.shields.io/badge/License-MIT-478CBF.svg?style=for-the-badge)](https://opensource.org/licenses/MIT)
[![Conduct](https://img.shields.io/badge/Conduct-Covenat%202.0-478CBF.svg?style=for-the-badge)](https://www.contributor-covenant.org/version/2/0/code_of_conduct/)
[![Made with C++20](https://img.shields.io/badge/C%2B%2B-20-478CBF?style=for-the-badge&logo=c%2B%2B&logoColor=white)](https://isocpp.org/)

- **Description**: A small game where colored "energy" units are moved between battery nodes until each battery is either empty or contains a single color.

## Download & Play

You can download native builds or play instantly in your browser.

- **Download native builds** — Get the latest Windows, linux or macOS builds from the Releases page: https://github.com/juan-medina/energy-swap/releases/
  Windows: Requires Microsoft Visual C\+\+ Redistributable (x64).
  macOS: Run `xattr -cr EnergySwap.app` before opening.

- **Play in browser (WebAssembly)** — Run the game without installing: https://juan-medina.github.io/energy-swap/
  Note: Use a modern browser with WebAssembly and WebGL support (Chrome, Edge, Firefox, Safari) for best results.

- If you want to use a controller in the browser version sometimes you need to:
	- Connect the controller and power it on before opening the browser.
	- Focus the browser window and press a button on the controller to "wake it up".
    - If still not working, close and reopen the browser.

Report bugs or feedback on the repository issues page: https://github.com/juan-medina/energy-swap/issues

**How it works (brief)**

- The main scene instantiates multiple battery scenes. Each battery holds up to 4 "energy" color units.
- Players click a battery to pick up a contiguous run of top-most energy units of the same color, then click another battery to place them if allowed by the rules.TB

**Credits**

- Developed by [Juan Medina](https://juan-medina.com).
- Music by [Abstraction](https://abstractionmusic.com/) (licensed as Public Domain CC-0).
- SFX by [Helton Yan & Beto Bezerra](https://heltonyan.itch.io/pixelcombat?download) (licensed under Attribution 4.0 International).
- Particle Pack by [Polar_34](https://polar-34.itch.io/particles) (license under MIT).
- Additional graphics by [Kenney.nl](https://kenney.nl/assets) (licensed under Public Domain CC-0).

## Engine

This project uses [pxe](https://github.com/juan-medina/pxe) — an opinionated game engine that provides the core framework and utilities for this game. (licensed under MIT)

pxe uses several third-party libraries:

- [raylib](https://www.raylib.com/) (licensed under zlib/libpng)
- [raygui](https://github.com/raysan5/raygui) (licensed under zlib/libpng)
- [spdlog](https://github.com/gabime/spdlog) (licensed under MIT)
- [Boxer](https://github.com/aaronmjacobs/Boxer) (licensed under MIT)
- [jsoncons](https://github.com/danielaparker/jsoncons) (licensed under Boost Software License 1.0)
- [PlatformFolders](https://github.com/sago007/PlatformFolders) (licensed under MIT)
- CRT Shader based on [Godot CRT Shader](https://perons.itch.io/godot-crt-shader) by perons.

**License**

- This project is licensed under the MIT License — see `LICENSE` for details.

**Contributing**

- Feel free to open issues or pull requests. For code changes, follow the existing style and keep changes focused.

**Contact**

- Author: Juan Medina (copyright 2026)
