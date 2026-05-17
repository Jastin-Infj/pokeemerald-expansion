# Weather Lab Terminal MVP Plan

## Entry Point Options

| Option | Pros | Cons | Decision |
|---|---|---|---|
| Weather Institute NPC / terminal | Thematically strong. | Story state and NPC placement need care. | TBD. |
| Debug map / debug menu | Low story risk. | Less polished. | Preferred for first runtime test. |
| Existing route object | Easy atmosphere preview. | Can confuse normal progression. | Not MVP unless safe. |

## Candidate Menu Options

- Clear
- Rain
- Thunderstorm
- Sandstorm
- Fog or Ash
- Cancel / Reset

## Reset Behavior

- First MVP may use `WEATHER_NONE` / Clear as reset.
- True map-default restore needs investigation before being promised.
- If default restore is not solved, document it as known risk.

## Future Work

- Unlock weather modes.
- Weather Institute reward.
- Atmosphere preview room.
- Map lighting preset preview.
- Script command wrapper for safe test-only use.
