# Tool Inventory Files

## Purpose

このページは、local tooling の復旧用 inventory file への入口である。JSON は machine-readable な保全情報として管理し、token や API key は placeholder のみ記録する。

## Files

| File | Contents |
|---|---|
| [`mcp_servers_inventory.json`](mcp_servers_inventory.json) | MCP server 名、command、args、用途、認証要否、確認済み制限。 |
| [`tool_dependency_inventory.json`](tool_dependency_inventory.json) | apt package、CLI tool、uv/npx tool、local artifact、任意追加候補。 |
| [`mgba_live_cache_preservation.md`](mgba_live_cache_preservation.md) | mGBA Live MCP cache の実体、backup 対象、復旧順、source clean 状態。 |

## Policy

- real API key、OAuth token、GitHub token、`SEMGREP_APP_TOKEN`、login URL の one-time token は書かない。
- local config の実値をそのまま貼らない。
- 復旧に必要な package 名、version、path、非 secret の args は記録してよい。
- 実際の local config と tracked docs は分離する。
