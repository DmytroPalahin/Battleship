# ADR 0005 — A test harness of our own

- **Status:** accepted
- **Date:** 2026-11-14

## Context

The engine needs unit tests that run everywhere the engine builds, including headless
containers. The usual C frameworks (Unity, CMocka, Check, GLib's GTest) each add a dependency
to download, vendor or install, and with it a licence to track and a version to maintain.

## Decision

Write the harness: `tests/bs_test.h`, about a hundred lines. It offers `TEST`, `CHECK`,
`CHECK_EQ`, `REQUIRE`, a table of cases and a `main` that prints TAP and filters cases by
name. Each test file is one executable registered with CTest.

## Consequences

- The engine and its tests build with a C11 compiler and nothing else.
- Everything a failing test prints can be traced by reading one short header.
- No fixtures, mocks or parameterised cases. The engine is made of pure functions on plain
  structures, so none of them has been missed. If that changes, this record gets superseded.
