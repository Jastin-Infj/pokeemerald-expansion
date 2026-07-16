# Box NPC Party Pool

Box NPC Party Pool is an implemented debug / test feature that lets an NPC
opponent use Pokemon copied from Pokemon Storage, starting with Box 1.

The first target is an isolated debug battle route: copy valid non-egg Pokemon
from Box 1 into a six-Pokemon candidate roster, choose three battle members for
singles or four for doubles, and battle without mutating the PC box. The feature
exists to make Smart Gimmick AI and future trainer AI tuning easier to test
against user-authored sets.

The stacked [Battle Team Boxes](../battle_team_boxes/README.md) extension adds
three persistent six-member rosters that may reference Pokemon from any Box and
uses those rosters as additional Box NPC candidate pools.

- [Goal](goal.md)
- [Implementation](implementation.md)
- [Test Plan](test_plan.md)
