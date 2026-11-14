# Project report

The report of the project, written in French with LaTeX.

```bash
make report                  # from the repository root
# or
cd docs/report && latexmk    # → build/battleship-report.pdf
```

It needs a TeX Live installation with `latexmk` (the `texlive-full` package on
Debian and Ubuntu, MacTeX on macOS). The PDF is not committed: every push rebuilds it in CI
([`docs.yml`](../../.github/workflows/docs.yml)) and each release carries it as
`battleship-report.pdf`.

| File                  | Content                                              |
| --------------------- | ---------------------------------------------------- |
| `main.tex`            | Document skeleton                                    |
| `preamble.tex`        | Packages, colours, styles                            |
| `titlepage.tex`       | Cover page                                           |
| `chapters/`           | One file per chapter                                 |
| `figures/`            | TikZ diagrams                                        |
| `data/benchmark.dat`  | Simulation results plotted in the AI chapter         |
| `references.bib`      | Bibliography                                         |

The numbers of the AI chapter are regenerated with:

```bash
battleship-cli simulate --csv --games 10000 --seed 1
```
