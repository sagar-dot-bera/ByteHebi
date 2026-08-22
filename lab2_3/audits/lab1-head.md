# Lab 2_3 — Code-Smell Audit

Audited commit: `f1b74f0` only. Report-only; no files were modified.

| Smell | Severity | Location | Why it is a genuine smell | Relevant refactoring technique / skill |
|---|---|---|---|---|
| Large Class | High | `includes/game.h:6` | `Game` owns game state/rules, two-player state, input, terminal lifecycle, rendering, dialogs, and high-score persistence—multiple independent reasons to change it. | Extract Class — `refactor-moving-features` |
| Data Clumps | Medium | `includes/game.h:49`, `includes/game.h:52`, `includes/game.h:61` | Player-specific state is represented as parallel fields (`snake`/`snake2`, `score`/`score2`, `playerName`/`player2Name`) and is repeatedly handled in paired branches. This indicates a missing player-state abstraction. | Extract Class / Introduce Parameter Object — `refactor-organizing-data` |
| Long Method | High | `source/game.cpp:44` | `Game::run()` combines terminal setup and teardown, size validation, dialog startup, tick scheduling, input, updates, and rendering in one method. | Extract Method — `refactor-composing-methods` |
| Long Method | High | `source/game.cpp:110` | `Game::processInput()` spans 218 lines and handles two-player name entry, dialog navigation/actions, both players’ controls, pause, restart, quit, and glyph-style selection. | Extract Method — `refactor-composing-methods` |
| Long Method | High | `source/game.cpp:329` | `Game::render()` spans 378 lines and renders board geometry, HUD, fruit, both snakes, glyph selection, and modal dialogs. These are independently changeable rendering units. | Extract Method — `refactor-composing-methods` |
| Long Method | High | `source/game.cpp:708` | `Game::update()` combines two-player collision detection, game-over state, scoring, high-score updates, fruit respawning, and both snakes’ movement. | Extract Method — `refactor-composing-methods` |
| Duplicate Code | High | `source/game.cpp:713`, `source/game.cpp:722`, `source/game.cpp:742`, `source/game.cpp:750` | Player-one and player-two collision and fruit/scoring branches repeat the same control flow with only field names and next-head variables changed. A rule change must be kept in both copies. | Extract Method / Form Template Method — `refactor-composing-methods` / `refactor-generalization` |
| Duplicate Code | High | `source/game.cpp:542`, `source/game.cpp:544`, `source/game.cpp:553`, `source/game.cpp:562`, `source/game.cpp:574` | Light, Heavy, Rounded, and DoubleLine snake styles repeat the same connectivity-condition ladder, differing only in glyph values. | Extract Method / Form Template Method — `refactor-composing-methods` / `refactor-generalization` |
| Dead Code | Low | `source/game.cpp:63` | `boardTW_double` is calculated but never read, so it adds no behaviour and misleadingly suggests an unused sizing path. | Delete unused code — `refactor-composing-methods` |
