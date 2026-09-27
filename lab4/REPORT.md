# Lab 4

| | |
|---|---|
| Repository | https://github.com/sagar-dot-bera/ByteHebi (ByteHebi, terminal snake on Notcurses) |
| Base tag | `lab4-base` at commit `b1702f0` |
| Pull request | ___ |

---

## 1. Five rules — [5]

Written before opening the source. Behaviour, with an observable outcome.

| # | Rule |
|---|---|
| 1 | Pressing the direction opposite to the one the snake is moving in is ignored; the snake keeps going the way it was going. |
| 2 | The snake dies (game over) when its head moves into a wall. |
| 3 | The snake dies (game over) when its head moves into its own body. |
| 4 | Eating a fruit adds exactly 10 to the score and makes the snake exactly one segment longer. |
| 5 | A fruit never appears on a cell the snake occupies. |

---

## 2. What you could test, and what stopped you — [10]

No source changes in this part. Every `file:line` below is a line in `lab4-base`.
Tests: `tests/partb_test.cpp`, linked against `source/snake.cpp` and `source/fruit.cpp` only.

| # | Rule | Test written? | Blocking dependency (`file:line` + what it is) |
|---|---|---|---|
| 1 | Opposite direction ignored | **Yes.** `Rule1_NoReverse.*` (2 tests, pass) | None. `snake.cpp:15` `Snake::setDirection` is public and `snake.cpp` includes only `<algorithm>`. |
| 2 | Dies at the wall | **No** | `game.cpp:746`: the wall check lives inside `Game::update()`, which is **private** (`game.h:37`). The result is written to `over`, which is private with no getter (`game.h:52`). `game.cpp:11` `#include <notcurses/notcurses.h>` means `game.cpp` does not compile in a test build without Notcurses (`fatal error: notcurses/notcurses.h: No such file or directory`). Even with Notcurses, constructing a `Game` calls `loadHighScore()` (`game.cpp:31`), and the destructor calls `saveHighScore()` (`game.cpp:40`) on the hard-coded path `"highscore.txt"` (`game.h:64`), so the test would overwrite the player's real high score. |
| 3 | Dies on own body | **No** (only the helper) | Same as rule 2: `game.cpp:754` in private `Game::update()` (`game.h:37`) sets private `over` (`game.cpp:756`). `Snake::hitsSelf` (`snake.cpp:49`) is public, but it only answers "is this cell in the body". Testing it does not test "the snake dies". |
| 4 | Fruit = +10 score, +1 length | **No** | `game.cpp:762-765` in private `Game::update()` (`game.h:37`); `score` is private with no getter (`game.h:51`). The fruit position is random and cannot be set: `fruit.cpp:7-9` seeds `std::mt19937` from `high_resolution_clock::now()`, and `Game`'s constructor places it (`game.cpp:34`). A test cannot put a fruit in front of the snake. `game.cpp:11` (Notcurses) blocks the build as in rule 2. |
| 5 | Fruit never on the snake | **Yes.** `Rule5_FruitNotOnSnake.*` (2 tests: 1 pass, **1 fails**) | Mostly unblocked: `Fruit::respawn` (`fruit.h:23`) takes the occupancy check as a parameter. The limit is `fruit.cpp:7-9`: the clock-seeded RNG makes the passing test different on every run (see §6). The failing test `FullBoardDoesNotPlaceFruitOnSnake` is a **real bug**. When no free cell is found in 1000 tries, `fruit.h:37` places the fruit at `{1,1}` without checking the snake, and on a full board that cell is snake. |

> **Rules testable without modifying the source: 2 / 5** (rules 1 and 5)

The whole game loop is untestable for one reason. All the rules that decide the game (death, scoring, growth) sit in one private method, in the same file as the terminal library.

---

## 3. Coverage, and what it missed — [6]

Only `snake.cpp`, `fruit.cpp` and the `fruit.h` template can be linked into a test (see §2), so `game.cpp` is not in the numbers. Its 834 lines have 0% coverage and could not even be compiled.

| | |
|---|---|
| Line coverage | **79.5 %** (58 / 73): `snake.cpp` 70.00% of 40, `fruit.cpp` 100% of 5, `fruit.h` 89.29% of 28 |
| Branch coverage | **45.9 %** taken at least once (28 / 61): `snake.cpp` 39.39% of 33, `fruit.h` 53.57% of 28 (gcov counts exception edges too) |
| Command used | `g++ -std=c++17 --coverage -O0 -g -I includes source/snake.cpp source/fruit.cpp tests/partb_test.cpp -o partb -lgtest -lgtest_main -pthread` → `./partb` → `gcov -b -o . partb-snake.gcda partb-fruit.gcda partb-partb_test.gcda` (MSYS2 UCRT64 g++ 16.1) |

**One rule that is executed by the suite but not verified by it:**

| | |
|---|---|
| Rule | **Rule 4, the "+1 length" half:** the snake grows by exactly one when it eats and otherwise keeps its length. |
| Line that runs | `snake.cpp:65-67`: `if (!grow) body.pop_back();`. It runs in `Rule1_NoReverse.OppositeDirectionIsIgnored` via `s.move()` at `tests/partb_test.cpp:13`. |
| The assertion that is missing | That test checks only the head position. Nothing checks `EXPECT_EQ(s.segments().size(), 3u)` after `move()`, and no test calls `move(true)` and checks `EXPECT_EQ(s.segments().size(), 4u)`. The `grow == true` branch is never taken. If you delete `pop_back()`, the snake grows on every step and the suite still passes. |

---

## 4. The seam — [10]

| | |
|---|---|
| Rule made testable | **Rule 4: eating a fruit adds exactly 10 to the score and grows the snake by one.** This was the hardest in Part B, with three separate blockers: private method, random fruit, and Notcurses in the same file. |
| Commit 1 (seam) | New `includes/rules.h` and `source/rules.cpp` with `advance(snake, fruit, width, height, score, highScore, respawnFruit)`, the body of `Game::update()` moved in unchanged. `Game::update()` now calls it (`source/game.cpp:743`). `Makefile` adds `source/rules.cpp` to `SRC`. |
| Commit 2 (test) | `tests/advance_test.cpp` (5 tests, rule 4), `tests/partb_test.cpp` (Part B), `tests/Makefile`, `lab4/REPORT.md`. No source file touched. |
| Seam kind | **Object seam.** `respawnFruit` is a `std::function`, so the call `respawnFruit(next)` inside `advance()` goes to whatever the caller passed in. The fruit position and the score also come in as parameters, so the clock-seeded RNG and the private fields are out of the way. Moving the rules into their own translation unit, with no Notcurses include, is what lets the test link. |
| Enabling point | The argument list of the `advance(...)` call in `Game::update()` (`source/game.cpp:743`). Production passes `fruit.position()` and a lambda that calls `fruit.respawn(...)`. The test passes a fixed `Point{11, 5}` and a spy. |
| What production code gave up | **Encapsulation of the score.** `score` and `highScore` are private to `Game`, but `advance()` takes them by non-const reference, so anything that can call a free function can now change them. The rule also no longer sits next to the state it guards. **A clear signature:** a no-argument member `update()` became a 7-parameter free function with two out-parameters, which is the *Long Parameter List* smell. **A direct call:** eating now goes through a type-erased `std::function` call on every tick. The lambda captures only `this`, so it fits libstdc++'s small buffer and nothing is allocated, and at one tick per 120 ms the cost is not measurable. **Nothing the player can see:** 5,000 random games (147,902 ticks, 2,246 fruits) produced identical snake, fruit, score and high score under the old and new code. Both versions were built and run against Notcurses 3.0.7. |

---

## 5. The double — [4]

| | |
|---|---|
| What you passed through the seam | **spy** (`RespawnSpy`, `tests/advance_test.cpp:7`) |
| The method under test | `advance()` (`source/rules.cpp`) |

`advance()` does not *ask* `respawnFruit` anything. It *tells* the game "the fruit was eaten, put a new one somewhere that is not `next`", and the call returns `void`. So there is no return value to check, and the only evidence the rule worked is that the call happened: once, with the cell the head moved into. `RespawnSpy` records the calls and the test asserts on them afterwards (`tests/advance_test.cpp:50-51`, and that there were none at `:65`). That makes it a spy, not a mock: the expectations live in the test, not inside the double. In `EatingAddsExactlyTenToScore` the same object is passed but never checked, so there it is only a dummy. The fruit `Point` is a plain value, not a double.

---

## 6. Two smells in your own tests — [5]

| | Smell | `file:line` | One-line fix |
|---|---|---|---|
| 1 | **The one not on the list: a test that (mostly) passes whatever the code does.** `RespawnAvoidsSnakeBody` puts a 3-cell snake on a 2,184-cell board. I replaced the occupancy check with `return false` (respawn ignores the snake) and ran it 2,000 times: it still **passed 1,502 times (75%)**. The RNG is clock-seeded (`fruit.cpp:7-9`), so a failure is not reproducible either. | `tests/partb_test.cpp:25` | Make the snake fill all but a handful of cells (for example, predicate "occupied unless x == 5") so a respawn that ignores the check fails on almost every draw. |
| 2 | **Eager test + assertion roulette.** One test checks four behaviours (result code, score, length, no respawn) with four bare `EXPECT`s and no messages. If it fails, the report says "line 64" and you have to read the test to learn which half of rule 4 broke. | `tests/advance_test.cpp:54` (asserts at `:62-65`) | Keep the shared arrangement, split the act into one test per behaviour (like the "eat" side already is), or add a `<<` message naming each quantity. |

Also noticed: `OppositeDirectionIsIgnored` (`tests/partb_test.cpp:8`) is a mild eager test. It asserts on the direction *and* then moves and asserts on the head. It is the same test that runs `snake.cpp:65-67` without checking length (§3).
