## Release Notes

### New Features

### Bug Fixes

### Improvements

- Better full-screen support on Mac.

### Known Issues

### Notes

- The Windows executable requires the Microsoft Visual C\+\+ Redistributable (x64). If you see runtime errors, install
  the latest Visual C\+\+ runtime: https://learn.microsoft.com/en-us/cpp/windows/latest-supported-vc-redist

- macOS users: Run `xattr -cr EnergySwap.app` in Terminal before opening the app.

- Run the game without installing at https://juan-medina.github.io/energy-swap/. Use a modern browser with WebAssembly
  and WebGL support (Chrome, Edge, Firefox, Safari) for best results.

- If you want to use a controller in the browser version sometimes you need to:
	- Connect the controller and power it on before opening the browser.
	- Focus the browser window and press a button on the controller to "wake it up".
	- If still not working, close and reopen the browser.