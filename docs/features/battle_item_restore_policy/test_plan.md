# Battle Item Restore Policy Test Plan

## Unit / Battle Tests

追加済み:

- `test/battle_item_restore.c`: `B_RESTORE_HELD_BATTLE_BERRIES == TRUE` で original Oran Berry が battle-end restore に含まれる。
- `test/battle_item_restore.c`: 既存の non-berry restore behavior が維持される。
- `test/battle/hold_effect/battle_item_restore.c`: full battle で Oran Berry を消費し、battle end 後に party held item へ戻る。

追加したい test:

- Sitrus Berry is consumed during a full battle but restored after battle under the new policy.
- Natural Gift consumes the berry during battle and battle-end policy restores the original item.
- Leppa Berry can still be restored by Recycle during battle.
- Harvest still restores a consumed berry during battle.
- Cud Chew still reuses the consumed berry on the next turn.
- Pickup can still pick up another battler's consumed item during battle.
- G-Max Replenish still restores target berries during battle.
- Belch remains usable after eating a berry, including after Recycle.
- Fling consumes the held item during battle and battle-end policy restores or preserves ownership according to the final policy.
- Bug Bite / Pluck eat target berries during battle and final ownership is documented.
- Thief / Covet / Pickpocket / Magician final ownership is documented for trainer and wild battles.
- Trick / Switcheroo / Bestow / Symbiosis final ownership is documented for trainer and facility battles.
- Corrosive Gas / Air Balloon exceptions are documented and tested if battle-end restore should not override them.
- Knock Off / Corrosive Gas / Air Balloon keep their intended non-restorable behavior if that policy is retained.

## Scenario Tests

- Trainer battle win with consumed berry.
- Trainer battle loss with `B_FLAG_NO_WHITEOUT`.
- Wild battle with consumed berry.
- Battle Frontier / Factory / Tent if the policy is enabled there.
- Battle selection enabled, selected subset consumes berry, original party slot receives restored item.

## Manual Checks

Confirmed:

- User confirmed in game that the related battle flow works and berry restoration works after battle.

Re-check if this area changes again:

- Bag quantity is not changed by held item restore.
- Held item icon in party menu is correct after battle.
- Summary screen held item is correct after battle.
- Link / recorded battle behavior is explicitly accepted or excluded.

## Validation Log

2026-05-08:

- `rtk make -j16 -O check TESTS=test/battle_item_restore.c`: passed.
- `rtk make -j16 -O all`: passed.
- `rtk make -j16 -O check`: passed. This includes mGBA headless through `mgba-rom-test-hydra`.
- `rtk make -j16 -O debug`: passed.
- Live mGBA focused runtime check:
  - `DISPLAY=:0 mgba-live-cli start` worked with the local Qt mGBA binary. MCP/offscreen wrapper startup did not become bridge-ready in this environment, so do not count offscreen startup as validated.
  - Reused local `.sav` candidates from `/tmp/mgba-aftercare-live`, but title screen showed only `NEW GAME` / `OPTION`; no `CONTINUE` was available.
  - Clean-start route reached field control, opened debug menu with `R+START`, then started `Party... > Start Debug Battle`.
  - Visible battle intro reached: `You are challenged by PKMN TRAINER Debugger!`; `gBattleTypeFlags` at `0x020000B8` read `12`.
  - Screenshot artifact: `/tmp/mgba-aftercare-live/debug-trainer-battle.png`.
- Focused Live mGBA item-screen/manual held item icon check was pending during the agent run. User later confirmed the real in-game behavior after push: the related battle flow and berry restore both worked.

2026-05-09:

- `rtk make -j16 -O all`: passed.
- `rtk make -j16 -O debug`: passed.
- `rtk make -j16 -O check TESTS=test/battle_item_restore.c`: passed, 2 tests.
- `rtk make -j16 -O check TESTS=test/battle/hold_effect/battle_item_restore.c`: passed.
- mGBA Live MCP check:
  - Initial `mgba_live_start` with default `/usr/games/mgba-qt` failed because that binary does not support `--script`.
  - `mgba_live_start` with `.cache/mgba-script-build-master/qt/mgba-qt` failed without display configuration.
  - `mgba_live_start` succeeded through `/tmp/mgba-live-display-wrapper`, which exports `DISPLAY=:0` and execs the script-capable mGBA binary.
  - `mgba_live_get_view` captured the title screen. `mgba_live_input_set` / `mgba_live_input_clear` accepted input and the next view reached the continue menu.
  - `mgba-live-cli stop` did not immediately mark the session stopped; the mGBA child appeared as a zombie under the MCP parent. Treat cleanup as stale-session cleanup risk, not as a failed runtime boot/input check.
  - Follow-up standardized `/home/jastin/.local/bin/mgba-qt` as the default wrapper. `mgba_live_start` without `mgba_path` then reached title screen, accepted A input, reached continue menu, and `mgba_live_stop` cleaned the session (`status --all` returned `[]`).
- GitHub Actions were not re-waited for this handoff. Local make and MCP evidence above are the current validation basis.
- User manual confirmation after push:
  - The related battle flow worked.
  - Berry restore worked in game.
  - Treat the item-restore slice as implementation-confirmed for this branch.
