# Simple Snake

A simple console-based Snake game written in C++ using POSIX termios for input.

## Features

- Grid-based playfield with walls
- Snake starts with length 3
- WASD or Arrow Keys to control
- Food spawns randomly; eating increases length and score
- Game over on self or wall collision
- Score displayed during play; restart or quit after game over

## Build

Linux/macOS (Terminal), or Windows via WSL/MSYS2/Cygwin:

```bash
g++ source/main.cpp source/game.cpp source/snake.cpp source/fruit.cpp -I includes -std=c++17 -O2 -o snake
```

Windows options:
- WSL (Ubuntu): use the Linux command above targeting your WSL path (e.g., /mnt/d/ByteHebi/... ).
- MSYS2/Cygwin: use the same g++ command in the MSYS2/Cygwin shell.

## Run

```bash
./snake
```

Controls: WASD or Arrow Keys. Press R to restart, Q to quit on game over.

Notes:
- Input uses POSIX termios in raw, non-blocking mode and ANSI escape codes for screen clearing. On native Windows cmd/PowerShell, build and run via WSL or MSYS2/Cygwin.



