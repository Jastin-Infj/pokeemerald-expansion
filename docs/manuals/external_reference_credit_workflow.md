# External Reference And Credit Workflow

## Document Metadata

| Field | Value |
|---|---|
| Last reviewed | 2026-05-10 |
| Baseline | `feature/prebattle-team-viewer` |
| Code status | Documentation-only workflow note |
| Provenance | User workflow guidance plus sampled public web references |

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
