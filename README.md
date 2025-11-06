# ByteHebi Snake (Linux, Notcurses)

ByteHebi is a Linux-only terminal Snake game built with Notcurses. It renders a colorful board with borders, a visible grid, and a side HUD for score and controls. The game opens with a name dialog and persists high scores between runs.

This README has two parts:
- For players: quick start, controls, tips
- For developers: build, project layout, tech choices, and trade-offs

## For players

### What you’ll see
- A bordered playfield with a subtle dotted grid so each cell is easy to see
- A side panel (HUD) with your name, current score, and high score
- A clean outer frame around the whole UI

### Controls
- Move: Arrow keys or WASD
- Pause/Resume: p or Space
- Quit: q
- Change snake style (cosmetic): g

### Flow
1) On start, an in-game dialog asks for your name. Press Enter to accept the default “Player” or type your name first.
2) Play: eat fruit (●) to grow and score points. Don’t hit the walls or yourself.
3) Game Over dialog shows your final score with options to Restart or Quit.

### Requirements
- Linux terminal with UTF‑8 and color support
- Terminal size at least 80x30 characters (default board size)

### How to run (prebuilt binary)
If you’ve already built it (or received a binary):

```bash
./snake
```

If the terminal is too small, the game will tell you to resize.

---

## For developers

### Why Notcurses?
Notcurses provides high-quality, modern terminal rendering (UTF‑8 glyphs, colors, compositing) without you micromanaging ANSI escape codes. We leverage it for:
- Unicode line art (borders, corners) and snake styles
- Smooth refresh via a retained-mode plane API
- Non-blocking input events (arrows, Enter, etc.)

Trade-offs:
- Linux-only target (easier to support well, fewer portability issues)
- Requires users to have Notcurses installed (libnotcurses-dev)

### Dependencies
- g++ (C++17)
- libnotcurses-dev
- pkg-config (recommended)

Install on Ubuntu/Debian:

```bash
sudo apt-get update
sudo apt-get install -y build-essential libnotcurses-dev pkg-config
```

### Build options

Using Makefile (recommended):

```bash
make
```

Manual compile (with pkg-config):

```bash
g++ source/main.cpp source/game.cpp source/snake.cpp source/fruit.cpp -I includes -std=c++17 -O2 $(pkg-config --cflags --libs notcurses) -o snake
```

Manual compile (without pkg-config fallback):

```bash
g++ source/main.cpp source/game.cpp source/snake.cpp source/fruit.cpp -I includes -std=c++17 -O2 -lnotcurses -lnotcurses-core -o snake
```

Run:

```bash
./snake
```

### Project layout

```
includes/
	fruit.h     # Fruit interface
	game.h      # Game loop, rendering, dialogs, HUD
	point.h     # Simple integer point
	snake.h     # Snake model & movement
source/
	fruit.cpp   # Fruit placement and respawn
	game.cpp    # Notcurses setup, input, update, render, dialogs
	main.cpp    # Entry point, default board size
	snake.cpp   # Snake behavior & direction logic
highscore.txt # Persistent high score file
Makefile      # Linux build (pkg-config for Notcurses)
README.md     # This file
```

### Key design choices & trade-offs

- Linux-only with Notcurses
	- Pros: Great terminal rendering, stable input handling
	- Cons: Requires Notcurses install; not portable to non-Linux terminals without extra work

- Unicode UI glyphs
	- Pros: Crisp borders, stylish snake segments, readable HUD
	- Cons: Needs UTF‑8 terminal, some fonts render differently

- Visible grid via dotted overlay
	- Pros: Clear cell boundaries without heavy backgrounds
	- Cons: Slightly more draw calls; we keep dots dim so the snake/fruit pop

- Modal dialogs (name, pause, game over)
	- Pros: Simple state management and clear user flow
	- Cons: Input focus is modal; no mouse support by design

### Configuration
- Default board size: 80x30 in `source/main.cpp`
- Default tick speed (difficulty): internal `tickMs` in `Game` (see `game.h`)
- High score file: `highscore.txt` in the working directory

### Troubleshooting
- Build errors about Notcurses
	- Ensure `libnotcurses-dev` and `pkg-config` are installed
	- Try the fallback link flags if `pkg-config` is unavailable

- “Terminal too small for configured game size.”
	- Resize your terminal window to at least 80 columns x 30 rows or adjust the defaults in `source/main.cpp`

### Windows note
This project targets Linux only. If you’re on Windows, use WSL and run the Linux commands from your project directory under `/mnt/...`.

---

Enjoy ByteHebi! PRs to improve UX, add snake styles, or refine the HUD are welcome as long as Linux + Notcurses remains the target.


