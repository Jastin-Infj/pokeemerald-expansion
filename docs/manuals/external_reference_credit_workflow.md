# External Reference And Credit Workflow

## Document Metadata

| Field | Value |
|---|---|
| Last reviewed | 2026-05-19 |
| Baseline | `docs/scout-selection-runtime-plan-20260519` |
| Code status | Documentation-only workflow note |
| Provenance | User workflow guidance, RHH credits, GitHub contributors, and recent merged PRs |

## Purpose

Use this workflow when investigating other Emerald / FireRed ROM hacks,
pokeemerald-expansion branches, UI references, graphics, screenshots, NPC art,
or community snippets.

The goal is to keep useful references discoverable without losing provenance.
Do not copy assets or code into this repo unless the license, source, author, and
credit requirements are clear.

## Source Priority

Prefer sources in this order:

1. Direct GitHub repository or branch.
2. Project-owned docs, wiki, release page, or README.
3. PokeCommunity / Hackdex / official project thread as discovery metadata.
4. Discord messages only when the project has an accessible MCP or the user
   provides screenshots / links with enough context.
5. Mirrors and pre-patched ROM pages only as secondary discovery clues.

Project names are often unreliable search keys. Search by credit names, GitHub
owners, Discord handles, file names, feature names, and exact UI terms. Credits
often point to the real author even when the public project title does not.
When a project title search fails, pivot to the author / credit handle and then
inspect that user's repositories, forks, gists, linked PokeCommunity posts, and
README credit sections.

## Search Pattern

For each candidate reference:

1. Capture the feature being investigated.
2. Search by project name and feature name.
3. Search by credited author / handle.
4. Search GitHub owners and forks directly.
5. Check README, `CREDITS`, release notes, and screenshots.
6. If only a mirror is found, use it to find the original thread / author, then
   treat the original as the source of record.
7. Record uncertainty instead of presenting a guess as fact.

When GitHub search is weak, inspect repository roots and common decomp paths
directly: `src/`, `include/`, `data/`, `graphics/`, `docs/`, `tools/`,
`CREDITS`, and `README`.

## Upstream Developer Roster

For pokeemerald-expansion work, generic search terms are often lower signal
than the people who repeatedly maintain, review, and merge similar changes.
Start from the upstream account, then pivot into that contributor's merged PRs,
commits, branches, forks, linked repos, and review comments.

Refresh this roster before treating it as current:

```sh
rtk gh api repos/rh-hideout/pokeemerald-expansion/contributors?per_page=40
rtk gh pr list -R rh-hideout/pokeemerald-expansion --state merged --limit 50 --json number,title,author,mergedAt,url
```

Use these as seed accounts, not as authority by themselves. A name on this list
does not prove authorship for a specific feature, and it does not grant license
permission to copy code or assets. Always keep the direct PR, commit, or credit
line with the feature docs when something becomes relevant.

| Account | Why to check first | Start points |
|---|---|---|
| [AsparagusEduardo](https://github.com/AsparagusEduardo) | Top upstream contributor and broad credits across code, data, docs, infrastructure, maintenance, reviews, tests, and project management. | [commits](https://github.com/rh-hideout/pokeemerald-expansion/commits?author=AsparagusEduardo), [merged PRs](https://github.com/rh-hideout/pokeemerald-expansion/pulls?q=is%3Apr+is%3Amerged+author%3AAsparagusEduardo) |
| [DizzyEggg](https://github.com/DizzyEggg) | Original expansion lineage and high-volume upstream contributor. Useful for older architectural patterns and inherited systems. | [commits](https://github.com/rh-hideout/pokeemerald-expansion/commits?author=DizzyEggg), [merged PRs](https://github.com/rh-hideout/pokeemerald-expansion/pulls?q=is%3Apr+is%3Amerged+author%3ADizzyEggg) |
| [GriffinRichards](https://github.com/GriffinRichards) | High-volume upstream contributor with deep decomp / engine history. | [commits](https://github.com/rh-hideout/pokeemerald-expansion/commits?author=GriffinRichards), [merged PRs](https://github.com/rh-hideout/pokeemerald-expansion/pulls?q=is%3Apr+is%3Amerged+author%3AGriffinRichards) |
| [AlexOn1ine](https://github.com/AlexOn1ine) | Maintainer / code contributor with frequent recent battle and mechanics PRs. | [commits](https://github.com/rh-hideout/pokeemerald-expansion/commits?author=AlexOn1ine), [merged PRs](https://github.com/rh-hideout/pokeemerald-expansion/pulls?q=is%3Apr+is%3Amerged+author%3AAlexOn1ine) |
| [hedara90](https://github.com/hedara90) | Maintainer / code contributor; recent upstream merge and test-oriented PR activity. | [commits](https://github.com/rh-hideout/pokeemerald-expansion/commits?author=hedara90), [merged PRs](https://github.com/rh-hideout/pokeemerald-expansion/pulls?q=is%3Apr+is%3Amerged+author%3Ahedara90) |
| [ghoulslash](https://github.com/ghoulslash) | Maintainer / code / design contributor; useful for debug utilities, gift flows, UI examples, and visual reference patterns. | [commits](https://github.com/rh-hideout/pokeemerald-expansion/commits?author=ghoulslash), [merged PRs](https://github.com/rh-hideout/pokeemerald-expansion/pulls?q=is%3Apr+is%3Amerged+author%3Aghoulslash) |
| [Bassoonian](https://github.com/Bassoonian) | Maintainer / code contributor with graphics and icon-related references in upstream history. | [commits](https://github.com/rh-hideout/pokeemerald-expansion/commits?author=Bassoonian), [merged PRs](https://github.com/rh-hideout/pokeemerald-expansion/pulls?q=is%3Apr+is%3Amerged+author%3ABassoonian) |
| [LOuroboros](https://github.com/LOuroboros) | High-volume contributor and wiki / how-to reference source. Useful as starter documentation context, but verify against current RHH source. | [commits](https://github.com/rh-hideout/pokeemerald-expansion/commits?author=LOuroboros), [wiki/repo search](https://github.com/search?q=user%3ALOuroboros+pokeemerald&type=repositories) |
| [PikalaxALT](https://github.com/PikalaxALT) | High-volume contributor with low-level decomp and upstream ecosystem history. | [commits](https://github.com/rh-hideout/pokeemerald-expansion/commits?author=PikalaxALT), [merged PRs](https://github.com/rh-hideout/pokeemerald-expansion/pulls?q=is%3Apr+is%3Amerged+author%3APikalaxALT) |
| [aarant](https://github.com/aarant) | High-volume upstream contributor. | [commits](https://github.com/rh-hideout/pokeemerald-expansion/commits?author=aarant), [merged PRs](https://github.com/rh-hideout/pokeemerald-expansion/pulls?q=is%3Apr+is%3Amerged+author%3Aaarant) |
| [Sierraffinity](https://github.com/Sierraffinity) | High-volume upstream contributor. | [commits](https://github.com/rh-hideout/pokeemerald-expansion/commits?author=Sierraffinity), [merged PRs](https://github.com/rh-hideout/pokeemerald-expansion/pulls?q=is%3Apr+is%3Amerged+author%3ASierraffinity) |
| [mrgriffin](https://github.com/mrgriffin) | Maintainer / code contributor in the RHH credits. | [commits](https://github.com/rh-hideout/pokeemerald-expansion/commits?author=mrgriffin), [merged PRs](https://github.com/rh-hideout/pokeemerald-expansion/pulls?q=is%3Apr+is%3Amerged+author%3Amrgriffin) |
| [Pawkkie](https://github.com/Pawkkie) | Maintainer / code / docs contributor, useful for docs and upstream workflow conventions. | [commits](https://github.com/rh-hideout/pokeemerald-expansion/commits?author=Pawkkie), [merged PRs](https://github.com/rh-hideout/pokeemerald-expansion/pulls?q=is%3Apr+is%3Amerged+author%3APawkkie) |
| [pkmnsnfrn](https://github.com/pkmnsnfrn) | Maintainer / code / project-management contributor. | [commits](https://github.com/rh-hideout/pokeemerald-expansion/commits?author=pkmnsnfrn), [merged PRs](https://github.com/rh-hideout/pokeemerald-expansion/pulls?q=is%3Apr+is%3Amerged+author%3Apkmnsnfrn) |
| [ExpoSeed](https://github.com/ExpoSeed) | Maintenance / review contributor in the RHH credits. Useful as a review-history pivot. | [commits](https://github.com/rh-hideout/pokeemerald-expansion/commits?author=ExpoSeed), [reviewed PRs](https://github.com/rh-hideout/pokeemerald-expansion/pulls?q=is%3Apr+reviewed-by%3AExpoSeed) |
| [kittenchilly](https://github.com/kittenchilly) | Code / research / data contributor; useful for data policy and Pokemon-facing UI references. | [commits](https://github.com/rh-hideout/pokeemerald-expansion/commits?author=kittenchilly), [merged PRs](https://github.com/rh-hideout/pokeemerald-expansion/pulls?q=is%3Apr+is%3Amerged+author%3Akittenchilly) |

Recent merged PR authors are also worth checking before broader web search,
especially when the feature touches active battle or field-script behavior:

| Account | Recent signal to refresh |
|---|---|
| [Cle-bit](https://github.com/Cle-bit) | Dense recent battle bugfix PRs. |
| [Bivurnum](https://github.com/Bivurnum) | Recent field / object-event behavior PRs. |
| [PhallenTree](https://github.com/PhallenTree) | Recent battle activation-order and message PRs. |
| [moostoet](https://github.com/moostoet) | Recent battle, overworld, and visual bugfix PRs. |
| [FosterProgramming](https://github.com/FosterProgramming) | Recent config / cleanup / trainer-define PRs. |
| [grintoul1](https://github.com/grintoul1) | Recent battle ability behavior PRs. |

Feature-specific pivots:

| Area | Accounts / sources to inspect first |
|---|---|
| Battle mechanics and tests | `AlexOn1ine`, `Cle-bit`, `AsparagusEduardo`, `PhallenTree`, `moostoet`, recent merged PRs for the same move / ability / item. |
| Field scripts, object events, NPC flow | `Bivurnum`, `hedara90`, `ghoulslash`, RHH merged PRs touching `data/maps/`, `event_object_movement`, or script macros. |
| Pokemon Summary / party-facing UI | RHH source first, then local Team Viewer feature branches, then `kittenchilly`, `Pawkkie`, `Bassoonian`, and merged PRs touching `pokemon_summary_screen.c` or `pokemon_icon.c`. |
| Data, config, generated tables | `AsparagusEduardo`, `Pawkkie`, `pkmnsnfrn`, `FosterProgramming`, and merged PRs touching `src/data/`, `include/config/`, or `tools/`. |
| Graphics, icons, visual assets | `ghoulslash`, `Bassoonian`, and the direct asset credit line in upstream `CREDITS.md`; do not copy image files without license / credit confirmation. |

## Asset Rules

- Do not download DeviantArt, Discord, or community-hosted art assets without
  explicit user approval.
- First record the URL, author / handle, license or usage note, preview context,
  and intended use.
- If the user downloads or approves an asset, record the local file path, source
  URL, author, required credit line, and any modification notes before wiring it
  into the build.
- If license / reuse terms are unclear, keep the asset out of source and use it
  only as visual reference until permission is confirmed.
- Add credit requirements to the owning feature docs before merge handoff.

## Discord

No Discord MCP is configured in the current Codex session. If one becomes
available, use it read-only for discovery unless the user explicitly asks for a
write action.

For Discord references, record:

- server / channel name if visible;
- message author / handle;
- message link or timestamp;
- attachment filename and preview description;
- whether the attachment was downloaded by the user or only referenced;
- credit / permission text if present.

If Discord cannot be accessed directly, ask the user for the message link,
screenshot, or attachment context rather than guessing.

## Seed References

These are discovery seeds, not automatic implementation sources:

| Reference | Use |
|---|---|
| <https://github.com/rh-hideout/pokeemerald-expansion> | Primary upstream base for this repo. Its README says it is built on pret's `pokeemerald`, includes `src/`, `include/`, `data/`, `graphics/`, `docs/`, and asks users to credit RHH and contributors. |
| <https://github.com/rh-hideout/pokeemerald-expansion/blob/master/CREDITS.md> | Upstream credit source for contributor identity and asset / code provenance. Use with recent PRs before generic web search. |
| <https://github.com/rh-hideout/pokeemerald-expansion/graphs/contributors> | Quick visual contributor-rank seed. Refresh with the GitHub API command above when choosing who to inspect. |
| <https://github.com/rh-hideout/pokeemerald-expansion/pulls?q=is%3Apr+is%3Amerged> | Recent merged PR stream. Use to identify currently active maintainers and specialists for a feature area. |
| <https://github.com/pret/pokeemerald> | Baseline Emerald decompilation reference. Useful for checking vanilla flow and file layout. |
| <https://www.pokecommunity.com/threads/the-pit-v2-roguelite-style-hack.528423/> | The Pit project thread seed. Use to chase original author / release context when accessible. |
| <https://www.hackdex.app/hack/the-pit> | The Pit discovery entry. Useful for screenshots, version, tags, and high-level feature clues. |
| <https://www.pokeharbor.com/2024/09/pokemon-the-pit-roguelite-style-hack/> | Mirror / article with credits and PokeCommunity source link. Treat as secondary until verified against the original source. |

## Documentation Output

When a reference becomes relevant to a feature, add a short table to that
feature's docs:

| Field | Required |
|---|---|
| Reference URL | Direct project source when possible |
| Author / handle | From credits, README, or thread |
| Feature observed | UI, code flow, asset style, etc. |
| Reuse type | code reference, visual reference, asset candidate, behavior reference |
| License / credit | required before copying |
| Verification status | confirmed, candidate, mirror-only, inaccessible |

Do not add unverified community code or art directly to implementation docs as
if it were authoritative.
