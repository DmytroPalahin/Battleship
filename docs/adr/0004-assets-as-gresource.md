# ADR 0004 — Assets compiled into the executable

- **Status:** accepted
- **Date:** 2026-11-14

## Context

Version 1 loaded its images with relative paths (`gtk_image_new_from_file("boom_44.png")`)
and the sound version with an absolute Windows path. The game only worked when started from
its own directory, on the machine it was written on.

## Decision

Bundle images, sounds and the style sheet with `glib-compile-resources` and link the result
into the executable. Code and CSS refer to them by resource path
(`/io/github/dmytropalahin/battleship/...`).

All styling moves to `assets/style.css`. C code only assigns class names; cell images are
CSS backgrounds selected by class.

## Consequences

- The program is a single file that runs from anywhere and installs trivially.
- No "file not found" at run time: a missing asset is a build error.
- Changing a picture requires a rebuild. Acceptable for a game of this size.
- The executable grows by the size of the assets: the release build weighs about 0.5 MB.
