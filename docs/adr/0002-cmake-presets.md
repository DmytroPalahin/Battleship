# ADR 0002 — CMake presets as the single source of build truth

- **Status:** accepted
- **Date:** 2026-11-14

## Context

Version 1 was built by a five-line Makefile that only worked from the source directory, with
`-Wall` as its only flag. Version 2 needs several configurations — development, release,
sanitizers, coverage, headless — and they must be identical on a laptop and in CI, or "works
on my machine" returns.

## Decision

Use CMake (3.25 or later) and describe every configuration in `CMakePresets.json`. CI runs
exactly `cmake --preset X && cmake --build --preset X && ctest --preset X`: no flag is spelled
out in a workflow file. Warnings, sanitizers and coverage are carried by an `INTERFACE`
target, `battleship_build_flags`, that project targets opt into.

A thin `Makefile` remains as a memo of the everyday commands (`make test`, `make run`…). It
only calls the presets.

## Consequences

- A CI failure is reproduced locally with one command.
- Flags never leak into generated or third-party code (the GResource bundle is compiled
  without them).
- CMake 3.25 is required. It ships with Ubuntu 24.04, Debian 12 and Homebrew.
