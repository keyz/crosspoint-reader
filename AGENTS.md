# AGENTS.md

## IMPORTANT (MUST FOLLOW)

The local dev environment has been bootstrapped via:

```bash
uv tool install "pioarduino-core @ https://github.com/pioarduino/platformio-core/archive/refs/tags/v6.1.19.zip"
uv venv --python 3.14
uv pip install --python .venv -r requirements.txt
```

You MUST make sure all python operations run in this venv. NEVER run a naked `python`, `python3`, or `pip`. Run every python operation via `uv run`, for example:

```bash
uv run python scripts/debugging_monitor.py /dev/cu.usbmodem101
```

As the only exception, you should run PlatformIO commands as `pio ...`; `pio` is installed as a uv tool, not in the repo's venv.

## Project Context

You may use [`CLAUDE.md`](./CLAUDE.md) for more project context. If you get conflicting instructions on how to invoke commands or install dependencies, you MUST follow the instructions here to keep all python operations in this venv.
