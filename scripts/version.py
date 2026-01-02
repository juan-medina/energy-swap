#!/usr/bin/env python3

# SPDX-FileCopyrightText: 2026 Juan Medina
# SPDX-License-Identifier: MIT

import json
import sys
from pathlib import Path

def main():
	if len(sys.argv) != 2:
		print("Usage: version.py <path-to-version-json>")
		sys.exit(1)

	path = Path(sys.argv[1])
	if not path.exists():
		print(f"Version file not found: {path}")
		sys.exit(1)

	data = json.loads(path.read_text())

	version = data.get("version", {})
	version["build"] = version.get("build", 0) + 1
	data["version"] = version

	path.write_text(json.dumps(data, indent=4))
	print(f"Updated build number to {version['build']}")

if __name__ == "__main__":
	main()
