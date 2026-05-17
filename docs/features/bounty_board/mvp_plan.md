# Bounty Board MVP Plan

## First Slice

- Fixed 3-request table.
- Item delivery only.
- No accepted-state tracking.
- Completion occurs on turn-in if the player has the required item.
- One-time reward guarded by event flag.
- Safe map entry point.

## Request Shape

| Field | Notes |
|---|---|
| Title | Short fixed text. |
| Required item | Existing item id, exact choice TBD. |
| Required quantity | Keep low. |
| Reward item | Existing item id. |
| Reward quantity | Keep low. |
| Completion flag | Needs explicit safe event flag allocation in runtime branch. |

## Future Work

- Rotating requests.
- Catch requests.
- Trainer battle requests.
- Region request board.
- Rank rewards.
- Story-gated requests.
