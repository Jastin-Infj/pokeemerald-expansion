# Trainer Titles / Achievement Badges Risks

## Runtime Risks

- Trainer Card UI is too risky for MVP.
- Persistent selected title requires save ownership.
- Unsafe event flag allocation.
- Fake achievements without real conditions.
- Duplicate grant.
- Text overflow.

## Risk Controls

- Avoid Trainer Card in MVP.
- Avoid active equipped title in MVP.
- Use small flag-only ownership model.
- Keep grant conditions simple and explicit.
