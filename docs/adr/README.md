# Architecture decision records

Short notes on the decisions that shaped the code: the context, what was chosen, and what it
costs. A decision is never edited after the fact; if it is reversed, a new record supersedes it.

| #                                                       | Decision                                              | Status   |
| ------------------------------------------------------- | ----------------------------------------------------- | -------- |
| [0001](0001-mvc-with-a-dependency-free-engine.md)       | Model / view / controller with a dependency-free engine | Accepted |
| [0002](0002-cmake-presets.md)                           | CMake presets as the single source of build truth     | Accepted |
| [0003](0003-stateless-ai-on-public-knowledge.md)        | A stateless AI fed with public knowledge only         | Accepted |
| [0004](0004-assets-as-gresource.md)                     | Assets compiled into the executable                   | Accepted |
| [0005](0005-in-tree-test-harness.md)                    | A test harness of our own                             | Accepted |
