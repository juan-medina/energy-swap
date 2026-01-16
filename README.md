# Energy Swap

[![License: MIT](https://img.shields.io/badge/License-MIT-478CBF.svg?style=for-the-badge)](https://opensource.org/licenses/MIT)
[![Conduct](https://img.shields.io/badge/Conduct-Covenat%202.0-478CBF.svg?style=for-the-badge)](https://www.contributor-covenant.org/version/2/0/code_of_conduct/)
[![Made with C++20](https://img.shields.io/badge/C%2B%2B-20-478CBF?style=for-the-badge&logo=c%2B%2B&logoColor=white)](https://isocpp.org/)

- **Description**: A small game where colored "energy" units are moved between battery nodes until each battery is either empty or contains a single color.

## Download & Play

You can either download a native Windows build or play instantly in your browser.

- **Download (Windows)** — Get the latest native Windows build from the Releases page: https://github.com/juan-medina/energy-swap/releases/
  Note: The Windows executable requires the Microsoft Visual C\+\+ Redistributable (x64). If you see runtime errors, install the latest Visual C\+\+ runtime: https://learn.microsoft.com/en-us/cpp/windows/latest-supported-vc-redist

- **Play in browser (WebAssembly)** — Run the game without installing: https://juan-medina.github.io/energy-swap/
  Note: Use a modern browser with WebAssembly and WebGL support (Chrome, Edge, Firefox, Safari) for best results.

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


**License**

- This project is licensed under the MIT License — see `LICENSE` for details.

**Contributing**

- Feel free to open issues or pull requests. For code changes, follow the existing style and keep changes focused.

**Contact**

- Author: Juan Medina (copyright 2026)
