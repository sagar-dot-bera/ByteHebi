# Lab 2_3 — Group Report

---

## 1. Tool and install route — [3]

| | |
|---|---|
| Agent used for run 2 | Codex CLI |
| `ubiquitous-language` install route | project `.agents/skills/ubiquitous-language` |
| `refactoring/` pack install route | project `.agents/skills/refactoring` |

The initial `.codex/skills` route was not discovered by Codex. The skills were then copied to the project-local `.agents/skills` directory, which Codex discovered successfully.

---

## 2. What I changed in the glossary — [4]

I changed the canonical term from "Playfield" to "Board" because the source code consistently uses "board" for the game area. I removed "Snake style" because it is a rendering-only concept rather than game-domain language. I added "Tick" because the game uses it as the synchronized unit for evaluating both snakes, and added "Game" because it is a first-class domain concept that owns the game state.

---

## 3. Smell delta — [6]

Reports: `lab2_3/audits/main.md` (the code as you received it) and `lab2_3/audits/lab1-head.md` (after your Lab-1 PR).

| | count | representative site (`file:line`) |
|---|---:|---|
| Smells my Lab-1 PR **introduced** | 3 | `includes/game.h:49`, `source/game.cpp:708`, `source/game.cpp:713` |
| Smells my Lab-1 PR **left untouched** | 6 | `includes/game.h:6` |
| Smells my Lab-1 PR **removed** | 0 | — |

---

## 4. Rejected candidates — [6]

| smell reported | `file:line` | why it does not hold |
|---|---|---|
| Long Method | `source/game.cpp:43` | `Game::run()` is a cohesive lifecycle orchestrator. It initializes the terminal, validates dimensions, runs the loop, and shuts down, while input, update, and rendering are delegated to separate methods. Its size alone does not establish a Long Method smell. |
| Data Clumps | `includes/game.h:49`, `includes/game.h:52`, `includes/game.h:61` | The audit inferred that the parallel player fields should become a Player abstraction, but it did not show the same variables repeatedly appearing together as required by the Data Clumps definition. |
| Long Method | `source/game.cpp:708` | `Game::update()` performs one cohesive game-tick transition: determining next positions, resolving collisions and fruit consumption, and moving both snakes in a defined order. The audit did not establish that these are independent responsibilities rather than one atomic update operation. |

---

## 5. Commit map — [7]

| # | sha | subject | what it is |
|---|---|---|---|
| 1 | 961a2db | docs: add ubiquitous language glossary | glossary |
| 2 | ce8a41e | docs: add code smell audits | smell report |
| 3 | 54d4499 | refactor: extract game input handling | **the refactor, alone** |
| 4 | daca9e8 | feat: restore multiplayer game | **the feature, alone** |

---

## 6. Two-run measurement — [4]

Run 1 is your Lab-1 branch — the numbers you already reported. Run 2 is commit 4 alone.

| | Run 1 (Lab 1) | Run 2 (commit 4) |
|---|---|---|
| Smells introduced | 3 | 0 |
| Lines changed, `git diff --shortstat -w` | 3 files changed, 362 insertions(+), 147 deletions(-) | 2 files changed, 105 insertions(+), 38 deletions(-) |
| Lines changed, **raw** (no `-w`) | 3 files changed, 416 insertions(+), 201 deletions(-) | 2 files changed, 107 insertions(+), 40 deletions(-) |
| Functions reached | 2 | 2 |
| Prompts to working code | 2 | 1 |
| Wall-clock time | 10 mins | 5 mins |

Commit 3 (the refactor) on its own: 2 files changed, 47 insertions(+), 33 deletions(-) lines `-w`, 2 files changed, 96 insertions(+), 82 deletions(-) raw.

---

## 7. Analysis Q1–Q2 — [5]

**Q1. Which smell did commit 3 actually fix?**
Commit 3 addressed the Long Method smell in `Game::processInput()` (`source/game.cpp:109`). Before commit 3, adding or modifying player input handling required editing a 200+ line monolithic method containing interleaved dialog, pause, quit, and single-player key checks. After commit 3 extracted directional input handling into modular helper callbacks, input processing became decoupled.

**Q2. Compare commit 4 to your Lab-1 diff.**
In Lab 1, adding multiplayer required 362 insertions and 147 deletions (`-w`) while introducing 3 new code smells (Data Clumps and Duplicate Code). In Run 2 (commit 4), building on commit 3's refactoring reduced the feature diff to 105 insertions and 38 deletions (`-w`) with 0 new smells introduced. The cost in lines changed and prompt iterations decreased significantly. What did not change was the boundary of 2 functions/files reached (`game.h` and `game.cpp`).

---

## 8. Analysis Q3–Q4 — [5]

**Q3. Go back through your Lab-1 `LLM-LOG.md`. Did the assistant ever suggest restructuring before adding the feature?**
No, the assistant in Lab 1 did not suggest restructuring. The prompt explicitly mandated implementing "in the most simple way without changing much... no renaming, no tidying up code you dislike." The LLM strictly followed instructions and wrote inline edits. To trigger pre-feature restructuring, the prompt would need to ask for a design audit first and explicitly require refactoring approval before code generation.

**Q4. How do you know commit 3 did not change behaviour?**
We do not know with mathematical certainty that commit 3 preserved exact behaviour; we only verified it by manually launching and playing the game. Because SnakeCycle lacks unit tests or automated regression suites for `Game`, manual playtesting was the sole verification method. To truly know, we would need automated unit tests for input dispatching and deterministic tick assertions.

---

## If you did not finish

N/A — completed all sections.
