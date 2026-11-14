# ADR 0003 — A stateless AI fed with public knowledge only

- **Status:** accepted
- **Date:** 2026-11-14

## Context

In version 1 the computer read the player's grid directly (`computer_cell->is_ship`) and kept
its "I am chasing a ship" memory in a global variable. That memory was easy to corrupt: when
the randomly chosen neighbour of a hit was off the board, the chase was dropped and the
wounded ship forgotten.

## Decision

The AI is one pure function:

```c
bs_status bs_ai_choose_target(bs_difficulty, const bs_knowledge *, bs_rng *, bs_coord *);
```

- It receives `bs_knowledge`: the state of each cell as an attacker sees it, and the lengths
  of the ships still afloat. Ship positions are not part of the structure.
- It keeps nothing between calls. "There is a wounded ship" is not remembered, it is read
  from the board (a `HIT` cell that is not `SUNK`).

Levels were chosen by **measuring** candidate strategies over thousands of seeded games, not
by intuition. The measurement showed that, with the no-touch rule, simply respecting what the
rules prove (medium) is worth about thirty shots, while a classic hunt-and-target routine
gains only seven more and a probability map three more. The levels were spaced accordingly:
96 / 66 / 55 shots on average.

## Consequences

- The AI cannot cheat, and a test cannot make it cheat by accident.
- Any position can be handed to it: that is how the targeting tests are written, and how the
  terminal `hint` command and the automated GUI test get their moves.
- Recomputing the density map on every shot costs a few thousand operations. Negligible.
- The hard level is close to the practical floor for this rule set: most of the remaining
  shots are spent looking for the four single-deck ships, which no reasoning can locate.
