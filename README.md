# ByteHebi Snake (Linux, Notcurses + POSIX)

Linux-only terminal Snake game using Notcurses for rendering and POSIX termios for nonblocking input.

## Features

- Grid with walls; snake grows on fruit
- Arrow keys or WASD
- Score and persistent high score (`highscore.txt`)
- Smooth TUI rendering with Notcurses (Linux/WSL)

## Build

Prerequisites (Ubuntu/Debian):

```bash
sudo apt-get update
sudo apt-get install -y build-essential libnotcurses-dev pkg-config
```

Compile (recommended with pkg-config to get all required libs):

```bash
g++ source/main.cpp source/game.cpp source/snake.cpp source/fruit.cpp -I includes -std=c++17 -O2 $(pkg-config --cflags --libs notcurses) -o snake
```

If pkg-config is unavailable, link both libraries explicitly:

```bash
g++ source/main.cpp source/game.cpp source/snake.cpp source/fruit.cpp -I includes -std=c++17 -O2 -lnotcurses -lnotcurses-core -o snake
```

This repo now targets Linux only. Use WSL if you’re on Windows and run the Linux commands above from your project directory under `/mnt/d/ByteHebi`.

## Run

```bash
./snake
```

Controls: Arrow keys or WASD. Press `q` to quit.

## Notes

- If your terminal is smaller than the configured game size (40x20 from `source/main.cpp`), the game will ask you to resize or adjust the width/height.
- High score is stored in `highscore.txt` in the working directory.


