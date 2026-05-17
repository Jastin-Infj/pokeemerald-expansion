# Weather Lab Terminal Investigation

## Questions

- Which scripts already change weather?
- Which functions apply weather immediately?
- Which weather constants are safe to expose?
- Can default map weather be restored?
- Which map is safest for an MVP entry point?

## Source Search Notes

Read-only search commands used during docs-only investigation:

```sh
rg "setweather|doweather|WEATHER_|SetWeather|weather" src include data/maps data/scripts
rg "WEATHER_" include data src | head -n 100
```

## Candidate Symbols / Areas

| Area | Candidate file / symbol | Confidence | Notes |
|---|---|---|---|
| Script commands | `data/script_cmd_table.inc` / `SCR_OP_SETWEATHER`, `SCR_OP_DOWEATHER` | High | Script opcodes map to `ScrCmd_setweather` and `ScrCmd_doweather`. |
| Script implementation | `src/scrcmd.c` / `ScrCmd_setweather`, `ScrCmd_doweather` | High | `ScrCmd_doweather` calls `DoCurrentWeather`. |
| Weather API | `include/field_weather.h` / `SetWeather`, `DoCurrentWeather`, `GetCurrentWeather` | High | Public weather entry points found. |
| Weather implementation | `src/field_weather_effect.c` / `SetWeather`, `DoCurrentWeather` | High | Runtime weather effect implementation. |
| Coord weather | `src/coord_event_weather.c` | Medium | Existing coord events call `SetWeather` for sunny, rain, thunderstorm, fog, ash, sandstorm, and more. |
| Existing scripts | `data/scripts/abnormal_weather.inc`, map `scripts.inc` files | Medium | Existing script examples use `setweather` and `doweather`. |

## Confirmed Weather Constants

Found in `include/constants/weather.h`:

- `WEATHER_NONE`
- `WEATHER_SUNNY`
- `WEATHER_RAIN`
- `WEATHER_RAIN_THUNDERSTORM`
- `WEATHER_FOG_HORIZONTAL`
- `WEATHER_VOLCANIC_ASH`
- `WEATHER_SANDSTORM`
- `WEATHER_SHADE`
- `WEATHER_DROUGHT`
- `WEATHER_DOWNPOUR`

Do not assume every weather constant is safe on every map. Some weather is story
or map-specific.
