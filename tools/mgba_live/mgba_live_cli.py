#!/usr/bin/env python3
"""Run mgba-live-cli with project-local session state.

The upstream CLI defaults to ``~/.mgba-live-mcp/runtime``. Keeping the runtime
root under the project cache makes direct Lua validation usable from a
workspace sandbox and keeps sessions from different worktrees separate.
"""

from __future__ import annotations

import os
from pathlib import Path

from mgba_live_mcp import live_cli


def configure_runtime_root() -> None:
    project_root = Path(__file__).resolve().parents[2]
    runtime_root = Path(
        os.environ.get(
            "MGBA_LIVE_RUNTIME_ROOT",
            str(project_root / ".cache" / "mgba-live-runtime"),
        )
    ).expanduser().resolve()

    live_cli.RUNTIME_ROOT = runtime_root
    live_cli.SESSIONS_DIR = runtime_root / "sessions"
    live_cli.ARCHIVED_SESSIONS_DIR = runtime_root / "archived_sessions"
    live_cli.ACTIVE_SESSION_FILE = runtime_root / "active_session"


if __name__ == "__main__":
    configure_runtime_root()
    live_cli.main()
