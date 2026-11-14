# Changelog

All notable changes to this project are documented in this file.

The format follows [Keep a Changelog](https://keepachangelog.com/en/1.1.0/) and the project
adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Changed

- The repository is now `DmytroPalahin/battleship`, in lower case like the programs and
  packages it builds. Links in the documentation, the report and the package metadata follow;
  addresses written with the former capital letter still resolve.
- Sound effects overlap instead of cutting each other short; at most four sound at once.

### Fixed

- Sound effects no longer fail when shots follow each other within milliseconds. Restarting a
  GStreamer pipeline whose threads were still running occasionally ended in an "Internal data
  stream error", which silenced that sound for the rest of the session and made the end-to-end
  GUI test fail (4 times in 240 stressed runs). Every occurrence of a sound now plays on a
  short-lived pipeline of its own that is never restarted.
- A sound that cannot be played is reported with the name of the failing GStreamer element.

## [2.1.0] - 2026-11-14

### Changed

- **All artwork and sounds are now original.** The third-party images and sound effects kept
  from the 2023 version are replaced by pictures drawn with ImageMagick and sounds synthesised
  with ffmpeg. The whole repository is now under the MIT licence, assets included.
- The assets shrink from 1.5 MB to 0.5 MB; the release build is correspondingly smaller.

### Added

- `tools/generate-assets.sh` regenerates every image and sound from scratch (`make assets`).

## [2.0.0] - 2026-11-14

A ground-up rewrite. The game is the same; everything underneath is new.

### Added

- **Game engine** (`battleship_core`): a dependency-free C11 library holding the rules — fleet
  placement, shots, turn order, score, end of game. No GTK, no dynamic allocation, no global
  state.
- **Terminal interface** (`battleship-cli play`): the same game on standard input and output.
- **AI simulator** (`battleship-cli simulate`): reproducible benchmark of the three levels,
  solo and head-to-head, with CSV output.
- **Hard level based on probabilities**: the computer counts every ship position consistent
  with what it knows and fires where they overlap most.
- **Seeded games**: `--seed N` replays a match exactly; the random generator is part of the
  engine instead of `rand()`.
- Coordinates (A–J, 1–10) around both boards and a live score under each of them.
- End-of-game dialog offering to replay, change level or quit; the remaining enemy ships are
  revealed when the game is over.
- Keyboard play: arrow keys move the missile cursor, <kbd>Space</kbd> fires, <kbd>F11</kbd>
  toggles full screen.
- Command-line options `--level`, `--seed`, `--mute`, `--version`.
- **Tests**: 54 unit cases, property tests replaying 2 250 full matches, CLI tests, and an
  end-to-end test that plays a whole game through the GTK widgets.
- **CI/CD** on GitHub Actions: build matrix (GCC, Clang, Apple Clang), sanitizers, coverage
  gate, clang-tidy, cppcheck, CodeQL, formatting check, headless build, and a release workflow
  producing packages, checksums and build provenance.
- **Documentation**: architecture guide, decision records, API comments, and the project
  report rewritten in LaTeX.
- Desktop entry, AppStream metadata and application icon; `.tar.gz` and `.deb` packages.

### Changed

- **Architecture**: one 2 666-line file became a model / view / controller design in three
  layers (engine, interfaces, application shell).
- **Build system**: CMake with presets replaces the hand-written Makefile.
- **Difficulty levels** are now defined by what the computer reasons about, and their strength
  is measured (96 / 66 / 55 shots on average to sink a fleet):
  - *Facile* fires at random;
  - *Moyen* also skips the cells the rules prove to be empty;
  - *Difficile* also finishes off wounded ships and hunts by probability.
- **One window**: the menu and the game are two pages of the same window instead of separate
  windows opened and closed in turn.
- **The computer plays one shot at a time**, half a second apart, instead of computing its
  whole turn inside the click handler.
- **Assets are compiled into the executable**; the game no longer has to be started from its
  own directory.
- **Styling moved to a style sheet** (`assets/style.css`); the C code only assigns class names.
- Fleet placement enumerates the legal positions of each ship and picks one, instead of
  drawing random positions until one fits.
- "Replace the fleet" redraws the player's fleet only.
- The window is resizable and no longer forced to full screen.
- French wording revised: accents on capitals, a few corrections.
- Sound effects are optional and selected at build time (GStreamer present or not).

### Fixed

Defects of version 1 that the rewrite removed:

- **Out-of-bounds read** in the hard level: the neighbour of a hit was read from the grid
  *before* checking that it was on the board.
- **Forgotten ships** in the hard level: when the neighbour drawn at random was off the board
  or already played, the computer dropped the chase and went back to random fire.
- **Nested event loops**: every "restart" called `gtk_main()` from inside a callback, one
  level deeper each time.
- **Frozen window** during the computer's turn, which ran entirely inside one event handler.
- **Hard-coded paths**: images loaded relative to the working directory, sounds from an
  absolute `D:\` path.
- **Leaks**: one CSS provider per label and one `GtkImage` per redraw, never released.
- **Biased randomness**: `rand() % n` replaced by an unbiased bounded generator.

### Removed

- The pre-built Windows executables and object files that were committed to the repository.
- The Code::Blocks project of the sound variant; sound is now a build option of the one
  code base.
- The background music track and the stock photograph used on the menu page (replaced by a
  generated background).
- The Word and PDF versions of the 2023 report from the source tree. The PDF is attached to
  the `v1.0.0` release; the report is now built from `docs/report/`.

## [1.0.0] - 2023-06-20

The original university project, by Dmytro Palahin and Ivanna Makam
(*Initiation aux interfaces graphiques*, Sup Galilée, 2022–2023).

### Added

- Battleship against the computer in a single GTK 3 program.
- Three difficulty levels.
- Random fleet placement with a "replace the fleet" button.
- Rules and trailer windows.
- A variant with sound effects through GStreamer.

[Unreleased]: https://github.com/DmytroPalahin/battleship/compare/v2.1.0...HEAD
[2.1.0]: https://github.com/DmytroPalahin/battleship/compare/v2.0.0...v2.1.0
[2.0.0]: https://github.com/DmytroPalahin/battleship/compare/v1.0.0...v2.0.0
[1.0.0]: https://github.com/DmytroPalahin/battleship/releases/tag/v1.0.0
