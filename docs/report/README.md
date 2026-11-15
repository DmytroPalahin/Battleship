# Project report

The report of the project, written with LaTeX, in two editions:

| Edition              | Main file                  | PDF                              |
| -------------------- | -------------------------- | -------------------------------- |
| French, the original | `battleship-report-fr.tex` | `build/battleship-report-fr.pdf` |
| English              | `battleship-report-en.tex` | `build/battleship-report-en.pdf` |

The English edition is the translation of the French one, chapter for chapter and with the
same labels, so that a change to one is easy to carry over to the other.

The PDFs of the latest release:
[English](https://github.com/DmytroPalahin/battleship/releases/latest/download/battleship-report-en.pdf),
[French](https://github.com/DmytroPalahin/battleship/releases/latest/download/battleship-report-fr.pdf).

```bash
make report                  # from the repository root
# or
cd docs/report && latexmk    # builds both editions into build/
```

It needs a TeX Live installation with `latexmk` (the `texlive-full` package on
Debian and Ubuntu, MacTeX on macOS). The PDFs are not committed: every push rebuilds them in CI
([`docs.yml`](../../.github/workflows/docs.yml)) and each release carries them.

| File                 | Content                                                       |
| -------------------- | ------------------------------------------------------------- |
| `fr/`, `en/`         | Cover page and one file per chapter, in each language         |
| `preamble.tex`       | Packages, colours, styles; picks the language-dependent texts |
| `references.bib`     | Bibliography, shared by both editions                         |
| `data/benchmark.dat` | Simulation results plotted in the AI chapter                  |
| `.latexmkrc`         | Build options and the list of documents                       |

Figures are drawn with TikZ inside the chapters; the screenshots come from
[`docs/screenshots`](../screenshots).

The numbers of the AI chapter are regenerated with:

```bash
battleship-cli simulate --csv --games 10000 --seed 1
```
