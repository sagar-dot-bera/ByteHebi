# Ubiquitous Language

This glossary is based on repository documentation and implemented game rules. The conversation supplied no additional game-domain vocabulary.

## Playfield and game pieces

| Term | Definition | Aliases to avoid | In code |
| --- | --- | --- | --- |
| **Playfield** | The bordered grid on which the two snakes and the fruit occupy cells. | Board, grid, screen | Playable interior - `source/main.cpp:6`; border - `source/game.cpp:349` |
| **Wall** | A border cell that causes a snake to lose when its next head position reaches it. | Edge, boundary | `source/game.cpp:715` |
| **Player** | One of two named participants, each controlling one snake and accumulating an individual score. | User, participant | `includes/game.h:52`, `includes/game.h:61`; `source/game.cpp:133` |
| **Snake** | A player's ordered chain of occupied cells, led by a head and ending in a tail. | Worm, player | `includes/snake.h:24` |
| **Head** | The leading cell of a snake, whose next position determines movement, eating, and collisions. | Front, current position | `includes/snake.h:24`, `includes/snake.h:31` |
| **Tail** | The last cell of a snake, removed on a move unless the snake grows. | Rear, end segment | `source/snake.cpp:64` |
| **Fruit** | The single shared item that appears on an unoccupied interior cell and can be eaten by a snake. | Food, pickup, dot | `includes/fruit.h:11`; `source/game.cpp:496` |

## Rules and score

| Term | Definition | Aliases to avoid | In code |
| --- | --- | --- | --- |
| **Direction** | One of up, down, left, or right, specifying the direction a snake will advance. | Heading, input key | `includes/snake.h:5`; `source/snake.cpp:28` |
| **Move** | One tick's advance in which a snake adds its next head cell and normally removes its tail cell. | Turn, step | `source/snake.cpp:61` |
| **Reverse direction** | A direction change directly opposite to the snake's current direction, which is ignored. | U-turn | `source/snake.cpp:15` |
| **Eat fruit** | A snake's next head position coinciding with the fruit position. | Collect fruit, consume food | `source/game.cpp:740` |
| **Growth** | Retaining the tail on a move after eating fruit, increasing the snake's length by one cell. | Length bonus, extension | `includes/snake.h:18`; `source/game.cpp:759` |
| **Score** | A player's current points total, increased by 10 when that player eats fruit. | Points, total | `source/game.cpp:742`, `source/game.cpp:750` |
| **High score** | The persisted highest individual player score observed by the game. | Best score, record | `source/game.cpp:745`, `source/game.cpp:788-804` |
| **Collision** | A next-head position on a wall, the moving snake's body, or the other snake's body. | Crash, hit | `source/game.cpp:713-728` |
| **Game over** | The state entered when at least one snake collides, presenting restart or quit choices. | Defeat, end screen | `source/game.cpp:731-735`; `source/game.cpp:701-702` |

## Session controls

| Term | Definition | Aliases to avoid | In code |
| --- | --- | --- | --- |
| **Pause** | A temporary state in which game ticks do not update the snakes. | Stop, suspend | `source/game.cpp:87-95`, `source/game.cpp:276-283` |
| **Resume** | Leaving pause and continuing the current game state. | Unpause, continue | `source/game.cpp:190-191` |
| **Restart** | Resetting both scores, both snakes, and the fruit to a new initial game while retaining names and high score. | New game, reset | `source/game.cpp:195-198`, `source/game.cpp:763-778` |
| **Quit** | Ending the current game loop. | Exit, close game | `source/game.cpp:174`, `source/game.cpp:271-274` |
| **Snake style** | A cosmetic glyph choice used to render snake bodies without changing any game rule. | Skin, mode, difficulty | `source/game.cpp:294-325`, `source/game.cpp:542-604` |

## Relationships

- A **Game** has exactly two **Players**, each controlling one **Snake** and owning one **Score**.
- A **Snake** has exactly one **Head**, zero or more body cells, and exactly one **Tail** while it exists.
- The **Playfield** contains one shared **Fruit**; its respawn position must not be occupied by either **Snake**.
- Eating the **Fruit** gives the eating **Player** 10 **Score** points and causes that player's **Snake** to undergo **Growth** on that **Move**.
- A **Collision** causes **Game over**; if both players collide, the code records Player 1 as the loser.

## Example dialogue

> **Dev:** "When Player 2 eats the shared **Fruit**, do both snakes grow?"

> **Domain expert:** "No. The **Fruit** is shared, but the player whose **Head** eats it receives 10 **Score** points and only that **Snake** undergoes **Growth**."

> **Dev:** "What ends the game?"

> **Domain expert:** "A **Collision** with a **Wall**, its own snake, or the other snake causes **Game over**. A **Restart** creates fresh snakes and fruit but retains the names and **High score**."

## Flagged ambiguities

- The README uses singular-player wording and says "Arrow keys or WASD" (`README.md:15-22`), while the code always creates two players and maps arrows to Player 1 but WASD to Player 2 (`source/game.cpp:233-268`). Canonical language: **two-player game**, **Player 1**, and **Player 2**; document controls per player.
- If both next heads reach the shared **Fruit** on the same tick, only Player 1 receives growth and points because the Player-2 branch is `else if` (`source/game.cpp:742-756`). The intended simultaneous-eating rule is not stated.
- If both players collide on the same tick, Player 1 is recorded as the sole loser due to `loser = p1dead ? 1 : 2` (`source/game.cpp:731-733`). The game does not represent a draw.
- **High score** could mean a session total or combined score, but the implementation persists the highest individual score across runs (`source/game.cpp:744-754`, `source/game.cpp:788-804`). Canonical meaning: highest individual score.
- **Difficulty** appears in comments/configuration (`includes/game.h:67`, `source/game.cpp:782-785`), but the game has one fixed tick interval and no player-selectable difficulty. Do not use **difficulty** as a current game-domain term.

## Code drift

Places where the code's name for a concept is not the canonical term. Reported, not renamed.

| Canonical term | Called in code | Location | Note |
| --- | --- | --- | --- |
| **Player 1 / Player 2** | `snake` / `snake2` | `includes/game.h:49-50` | Generic object names obscure that each snake is player-owned. |
| **Player 1 / Player 2 controls** | README's "Arrow keys or WASD" | `README.md:15` | The prose implies alternatives for one player; code assigns them to different players. |
| **Two-player game** | Singular-player prose | `README.md:20-23` | Startup collects two names and maintains two scores. |
| **High score** | *(not fully described)* | `README.md:3`, `source/game.cpp:744-754` | The README does not say it is an individual, cross-run maximum. |
