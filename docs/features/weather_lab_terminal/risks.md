# Weather Lab Terminal Risks

## Runtime Risks

- Weather not refreshing immediately.
- Story weather accidentally overwritten.
- Map transition restoring unexpected weather.
- Battle transition interactions.
- Weather constants that are not safe outside specific maps.
- Player confusion if weather persists longer than expected.

## Risk Controls

- Use an isolated debug / test entry first.
- Expose only confirmed existing weather constants.
- Keep default restore as TBD until tested.
- Do not store custom weather in save data.
- Do not modify story abnormal weather scripts in MVP.
