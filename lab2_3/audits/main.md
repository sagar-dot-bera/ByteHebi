# Lab 2_3 — Code-Smell Audit

Audited commit: `b1702f0` only. Report-only; no files were modified.

| Smell | Severity | Location | Why it is a genuine smell | Relevant refactoring technique / skill |
|---|---|---|---|---|
| Large Class | High | `includes/game.h:6` | `Game` owns game rules/state, input handling, terminal lifecycle, all rendering, dialog behaviour, difficulty settings, and high-score persistence. These are independent reasons to change the class. | Extract Class — `refactor-moving-features` |
| Long Method | High | `source/game.cpp:43` | `Game::run()` combines terminal setup/teardown, terminal-size validation, dialog startup, tick scheduling, input dispatch, game updates, and rendering in one 64-line method. | Extract Method — `refactor-composing-methods` |
| Long Method | High | `source/game.cpp:109` | `Game::processInput()` is over 200 lines and handles name-entry editing, dialog navigation/actions, gameplay controls, pause state, restart, quit, and glyph-style selection. Its distinct responsibilities make changes difficult to isolate. | Extract Method — `refactor-composing-methods` |
| Long Method | High | `source/game.cpp:315` | `Game::render()` spans 424 lines and renders board geometry, grid, HUD, frames, fruit, snake styles/gradients, and modal dialogs. The method contains several cohesive rendering units that can change independently. | Extract Method — `refactor-composing-methods` |
| Duplicate Code | High | `source/game.cpp:547`, `source/game.cpp:549`, `source/game.cpp:565`, `source/game.cpp:581`, `source/game.cpp:601` | The Light, Heavy, Rounded, and DoubleLine cases repeat the same connectivity-condition ladder; only the glyph values differ. Any correction or new connectivity rule must be replicated across each case. | Extract Method / Form Template Method — `refactor-composing-methods` / `refactor-generalization` |
| Dead Code | Low | `source/game.cpp:62` | `boardTW_double` is calculated but never read. It adds a misleading apparent sizing path without affecting behaviour. | Delete unused code — `refactor-composing-methods` |
