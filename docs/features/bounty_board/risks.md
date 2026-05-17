# Bounty Board Risks

## Runtime Risks

- Duplicate reward if flag handling is wrong.
- Removing wrong item quantity.
- Bag full reward failure.
- Using unsafe flags.
- Map NPC placement conflict.
- Player confusion if no accepted state exists.
- Scope creep into catch / battle hooks.

## Risk Controls

- Use only fixed item delivery in MVP.
- Do not allocate flags until runtime branch.
- Check item space before removing required items if reward can fail.
- Keep battle / catch hooks out of first slice.
