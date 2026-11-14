# Contributing

Thank you for taking the time to contribute. This page tells you how to get a working tree,
what the continuous integration will check, and how changes are proposed.

## Getting set up

You need a C11 compiler, CMake ≥ 3.25, pkg-config and the GTK 3 development files
(see the [README](README.md#quick-start) for the package names). Then:

```bash
make test        # configure, build and run the tests (preset "dev")
make run         # start the game
```

Useful extras, all optional:

| Tool                        | Used by                    |
| --------------------------- | -------------------------- |
| `clang-format` **23**       | `make format`              |
| `clang-tidy`, `cppcheck`    | `make lint`                |
| `gcovr`                     | `make coverage`            |
| TeX Live with `latexmk`     | `make report`              |
| Doxygen                     | `make docs`                |
| ImageMagick, ffmpeg, pngquant | `make assets`            |

## Before you open a pull request

Run what CI runs. If these pass locally, the pipeline will pass too:

```bash
make format      # reformat the sources
make test        # unit, property and CLI tests
make asan        # the same under AddressSanitizer and UBSan
make lint        # clang-tidy and cppcheck
```

A change is ready when:

- [ ] it builds without warnings (warnings are errors in every preset);
- [ ] new behaviour comes with a test, and a bug fix with a test that failed before it;
- [ ] `docs/` and `CHANGELOG.md` (section *Unreleased*) reflect it when it is user-visible;
- [ ] the engine still builds without GTK (`make test PRESET=headless`).

## Where things go

The one architectural rule is that **dependencies point inwards**
([docs/architecture.md](docs/architecture.md)):

- a rule of the game belongs in `src/core/` and must not include GTK or GLib;
- a widget belongs in `src/ui/view.c` and must not decide anything about the game;
- `src/ui/controller.c` connects the two and holds neither a rule nor a widget.

If a change does not fit, say so in the pull request — that is a design discussion worth
having, and its outcome is recorded as a [decision record](docs/adr).

## Code style

- C11, no compiler extensions. Formatting is whatever `clang-format` produces with the
  repository's `.clang-format`; it is not up for debate in review.
- Public names are prefixed `bs_` (functions, types) or `BS_` (macros, enumerators).
- Functions that can be refused return a `bs_status` and leave their arguments untouched on
  error. Programming errors that cannot be reported are `assert`ed.
- No global mutable state in the engine. Randomness comes from a `bs_rng` argument.
- Comments explain *why*. The public headers are the reference documentation: every
  declaration has a comment stating its contract.
- User-visible text goes in `src/ui/texts.h`, never in widget code.
- Images and sounds are generated: change `tools/generate-assets.sh`, not the files in
  `assets/`. Only original material is accepted; nothing downloaded.

## Commits

Commits follow [Conventional Commits](https://www.conventionalcommits.org):

```
<type>(<scope>): <summary in the imperative, lower case, no full stop>

Why the change is needed and what it does, wrapped at 72 columns.
```

Types in use: `feat`, `fix`, `refactor`, `perf`, `test`, `docs`, `build`, `ci`, `chore`.
Scopes: `core`, `ai`, `cli`, `ui`, `assets`, `report`.

Keep each commit a single logical change that builds and passes the tests on its own.

## Releases

1. Move the *Unreleased* entries of `CHANGELOG.md` under a new version heading.
2. Update the version in `CMakeLists.txt`, `CITATION.cff` and the AppStream metadata.
3. Tag the commit `vX.Y.Z` and push the tag. The release workflow does the rest.

## Reporting bugs and proposing features

Use the issue templates. For a bug, the seed of the game (`--seed`) and the level are the two
most useful things you can give: together they reproduce the match exactly.

Security problems should not be reported in public issues: see [SECURITY.md](SECURITY.md).
