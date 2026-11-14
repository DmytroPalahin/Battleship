# ADR 0001 — Model / view / controller with a dependency-free engine

- **Status:** accepted
- **Date:** 2026-11-14

## Context

Version 1 was a single 2 666-line file. Game state lived in global variables and inside GTK
widgets at the same time (`Cell` held both `is_ship` and a `GtkWidget *`), so the rules could
not run without a window. The three difficulty levels were three copies of the same 400-line
click handler; a fix made in one copy was missing from the others.

The course the project was written for teaches the remedy: a *model* library that knows the
rules, a *view* library that knows the widgets, and a *controller* that connects them.

## Decision

Split the program along those three lines, and make the split physical:

- the model is a static library, `battleship_core`, in plain C11, with **no** dependency
  beyond the standard library and **no** dynamic allocation;
- the view exposes no `GtkWidget`: it is driven in game vocabulary and reports user gestures
  through a table of callbacks;
- the controller holds no rule and no widget; after every change it re-synchronises the whole
  view from the model.

A second front-end, `battleship-cli`, is built on the same model.

## Consequences

- The rules are tested without a display, in milliseconds, under sanitizers.
- One code path serves the three levels; a level is a value, not a copy.
- The headless CI job proves the dependency rule on every commit.
- Synchronising everything after each move does more work than strictly needed (200 cells).
  At this scale it is not measurable, and it removes a whole class of "stale widget" bugs.
- More files, more indirection: a newcomer has to read `docs/architecture.md` first.
