# Future Runtime Handoff: Weather Lab Terminal

FUTURE USE ONLY.

Do not use this prompt during the docs-only task.

```text
runtime implementation task.

Goal:
Implement Weather Lab Terminal MVP. Use existing weather command / functions to
create a debug or test weather-switching terminal.

branch:
feature/weather-lab-terminal-mvp

Docs:
- docs/features/weather_lab_terminal/*

Investigation:
rg "setweather|doweather|WEATHER_|SetWeather|weather" src include data/maps data/scripts

MVP:
- Existing weather only.
- No new weather effects.
- No save data.
- Add terminal NPC / script to Weather Institute or debug/test map.
- Menu: Clear, Rain, Thunderstorm, Sandstorm, Fog / Ash / existing safe weather, Cancel / Reset.
- Apply weather on current map.
- If map default weather restore is hard, make Reset conservative or record known risk.

Forbidden:
- Do not rewrite the map weather system.
- Do not mix with runtime rule options.
- Do not store weather state in SaveBlock.
- Do not break story weather events.

Validation:
rtk mdbook build docs
rtk make -j16 -O all
rtk make -j16 -O debug
rtk make -j16 -O check

mGBA:
- Open terminal.
- Switch Clear / Rain / Thunderstorm.
- Close with Cancel.
- Confirm map transition does not break.
- Confirm battle transition does not break.
```
