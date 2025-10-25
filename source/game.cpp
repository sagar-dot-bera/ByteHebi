// Clean, cross-platform implementation of Game using Windows or POSIX input

#include "game.h"
#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <chrono>
#include <cstdio>
#include <limits>

#ifdef _WIN32
#include <windows.h>
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>
#endif

static constexpr char WALL_CHAR = '#';
static constexpr char SNAKE_HEAD_CHAR = 'O';
static constexpr char SNAKE_BODY_CHAR = 'o';
static constexpr char FRUIT_CHAR = '*';

#ifndef _WIN32
// Terminal configuration state (POSIX)
static termios g_origTerm;
static int g_origFlags = -1;
static bool g_termConfigured = false;
#else
// Console input configuration state (Windows)
static HANDLE g_hIn = INVALID_HANDLE_VALUE;
static DWORD g_origInMode = 0;
static bool g_inputConfigured = false;
#endif

Game::Game(int width, int height, const std::string &name)
    : width(width), height(height),
      snake(width / 2, height / 2, 3), fruit(width, height)
{
    playerName = name;
    // place initial fruit not overlapping snake
    fruit.respawn([&](const Point &p)
                  { return snake.contains(p); });

#ifndef _WIN32
    // Set terminal to raw, non-blocking mode for immediate key reads (POSIX)
    if (!g_termConfigured)
    {
        if (tcgetattr(STDIN_FILENO, &g_origTerm) == 0)
        {
            termios raw = g_origTerm;
            raw.c_lflag &= ~(ICANON | ECHO);
            raw.c_cc[VMIN] = 0;
            raw.c_cc[VTIME] = 0;
            tcsetattr(STDIN_FILENO, TCSANOW, &raw);
            g_origFlags = fcntl(STDIN_FILENO, F_GETFL, 0);
            if (g_origFlags != -1)
            {
                fcntl(STDIN_FILENO, F_SETFL, g_origFlags | O_NONBLOCK);
            }
            g_termConfigured = true;
        }
    }
#else
    // Configure Windows console input: disable Quick Edit to avoid input freezing
    if (!g_inputConfigured)
    {
        g_hIn = GetStdHandle(STD_INPUT_HANDLE);
        if (g_hIn != INVALID_HANDLE_VALUE && GetConsoleMode(g_hIn, &g_origInMode))
        {
            DWORD mode = g_origInMode;
            // To change extended flags, you must set ENABLE_EXTENDED_FLAGS first
            SetConsoleMode(g_hIn, mode | ENABLE_EXTENDED_FLAGS);
            mode &= ~ENABLE_QUICK_EDIT_MODE; // disable quick edit so mouse selection won't freeze app
            SetConsoleMode(g_hIn, mode | ENABLE_EXTENDED_FLAGS);
            g_inputConfigured = true;
        }
    }
#endif
}

Game::~Game()
{
#ifndef _WIN32
    // Restore terminal settings if we changed them (POSIX)
    if (g_termConfigured)
    {
        tcsetattr(STDIN_FILENO, TCSANOW, &g_origTerm);
        if (g_origFlags != -1)
        {
            fcntl(STDIN_FILENO, F_SETFL, g_origFlags);
        }
        g_termConfigured = false;
    }
#else
    // Restore Windows console input mode
    if (g_inputConfigured && g_hIn != INVALID_HANDLE_VALUE)
    {
        SetConsoleMode(g_hIn, g_origInMode);
        g_inputConfigured = false;
    }
#endif
}

void Game::reset()
{
    snake = Snake(width / 2, height / 2, 3);
    score = 0;
    over = false;
    exitRequested = false;
    fruit = Fruit(width, height);
    fruit.respawn([&](const Point &p)
                  { return snake.contains(p); });
}

void Game::processInput()
{
#ifdef _WIN32
    // Windows: read console input events (non-blocking)
    if (g_hIn == INVALID_HANDLE_VALUE)
        return;
    INPUT_RECORD records[16];
    DWORD count = 0;
    while (PeekConsoleInput(g_hIn, records, 16, &count) && count > 0)
    {
        // Remove them from the queue
        if (!ReadConsoleInput(g_hIn, records, count, &count))
            break;
        for (DWORD i = 0; i < count; ++i)
        {
            const INPUT_RECORD &rec = records[i];
            if (rec.EventType == KEY_EVENT && rec.Event.KeyEvent.bKeyDown)
            {
                WORD vk = rec.Event.KeyEvent.wVirtualKeyCode;
                switch (vk)
                {
                case VK_UP:
                case 'W':
                    snake.setDirection(Direction::Up);
                    break;
                case VK_DOWN:
                case 'S':
                    snake.setDirection(Direction::Down);
                    break;
                case VK_LEFT:
                case 'A':
                    snake.setDirection(Direction::Left);
                    break;
                case VK_RIGHT:
                case 'D':
                    snake.setDirection(Direction::Right);
                    break;
                case 'X':
                    over = true;
                    exitRequested = true;
                    break;
                default:
                    break;
                }
            }
        }
        // Check if more events are pending
        count = 0;
    }
    // Fallback: also poll keys in case events are not delivered in this console
    auto isKey = [](int vk)
    { return (GetAsyncKeyState(vk) & 0x8000) != 0; };
    if (isKey(VK_UP) || isKey('W'))
        snake.setDirection(Direction::Up);
    else if (isKey(VK_DOWN) || isKey('S'))
        snake.setDirection(Direction::Down);
    else if (isKey(VK_LEFT) || isKey('A'))
        snake.setDirection(Direction::Left);
    else if (isKey(VK_RIGHT) || isKey('D'))
        snake.setDirection(Direction::Right);
    if (isKey('X'))
    {
        over = true;
        exitRequested = true;
    }
#else
    // POSIX/WSL: use select() to read available input bytes and parse sequences
    while (true)
    {
        fd_set set;
        FD_ZERO(&set);
        FD_SET(STDIN_FILENO, &set);
        timeval tv{0, 0};
        int rv = select(STDIN_FILENO + 1, &set, nullptr, nullptr, &tv);
        if (rv <= 0)
            break; // no more input available this frame

        unsigned char buf[16];
        ssize_t n = ::read(STDIN_FILENO, buf, sizeof(buf));
        if (n <= 0)
            break;

        for (ssize_t i = 0; i < n; ++i)
        {
            unsigned char ch = buf[i];
            if (ch == 27) // ESC
            {
                // Try to read two more bytes for ESC [ A/B/C/D or ESC O A/B/C/D
                unsigned char seq[2];
                int got = 0;
                for (int j = 0; j < 2; ++j)
                {
                    fd_set set2;
                    FD_ZERO(&set2);
                    FD_SET(STDIN_FILENO, &set2);
                    timeval tv2{0, 10000}; // wait up to 10ms per byte
                    int rv2 = select(STDIN_FILENO + 1, &set2, nullptr, nullptr, &tv2);
                    if (rv2 > 0)
                    {
                        unsigned char c;
                        if (::read(STDIN_FILENO, &c, 1) == 1)
                            seq[got++] = c;
                    }
                }
                if (got >= 2 && (seq[0] == '[' || seq[0] == 'O'))
                {
                    switch (seq[1])
                    {
                    case 'A':
                        snake.setDirection(Direction::Up);
                        break;
                    case 'B':
                        snake.setDirection(Direction::Down);
                        break;
                    case 'D':
                        snake.setDirection(Direction::Left);
                        break;
                    case 'C':
                        snake.setDirection(Direction::Right);
                        break;
                    default:
                        break;
                    }
                }
            }
            else
            {
                switch (ch)
                {
                case 'w':
                case 'W':
                    snake.setDirection(Direction::Up);
                    break;
                case 's':
                case 'S':
                    snake.setDirection(Direction::Down);
                    break;
                case 'a':
                case 'A':
                    snake.setDirection(Direction::Left);
                    break;
                case 'd':
                case 'D':
                    snake.setDirection(Direction::Right);
                    break;
                case 'x':
                case 'X':
                    over = true;
                    exitRequested = true;
                    break;
                default:
                    break;
                }
            }
        }
    }
#endif
}

void Game::update()
{
    Point next = snake.nextHead();
    // Boundary collision
    if (next.x <= 0 || next.x >= width - 1 || next.y <= 0 || next.y >= height - 1)
    {
        over = true;
        return;
    }
    // Self collision
    if (snake.hitsSelf(next))
    {
        over = true;
        return;
    }

    bool grow = (next == fruit.position());
    snake.move(grow);
    if (grow)
    {
        score += 1;
        fruit.respawn([&](const Point &p)
                      { return snake.contains(p); });
    }
}

void Game::render() const
{
#ifdef _WIN32
    system("cls");
#else
    // ANSI clear: erase screen and move cursor to home
    std::cout << "\x1b[2J\x1b[H";
#endif

    std::vector<std::string> grid(height, std::string(width, ' '));

    // Walls
    for (int x = 0; x < width; ++x)
    {
        grid[0][x] = WALL_CHAR;
        grid[height - 1][x] = WALL_CHAR;
    }
    for (int y = 0; y < height; ++y)
    {
        grid[y][0] = WALL_CHAR;
        grid[y][width - 1] = WALL_CHAR;
    }

    // Fruit
    const auto &fp = fruit.position();
    grid[fp.y][fp.x] = FRUIT_CHAR;

    // Snake
    bool first = true;
    for (const auto &seg : snake.segments())
    {
        grid[seg.y][seg.x] = first ? SNAKE_HEAD_CHAR : SNAKE_BODY_CHAR;
        first = false;
    }

    // Print
    for (int y = 0; y < height; ++y)
    {
        std::cout << grid[y] << "\n";
    }
    std::cout << playerName << "'s Score: " << score << "\n";
    std::cout << "Controls: WASD or Arrow Keys. Press X to exit.\n";
}

void Game::gameOverScreen() const
{
    std::cout << "\nGame Over! Final Score: " << score << "\n";
    std::cout << "Press R to restart or Q to quit..." << std::endl;
}

int Game::run()
{
    chooseDifficulty();
    while (true)
    {
        // Main game session
        reset();
        while (!over)
        {
            processInput();
            update();
            render();
            std::this_thread::sleep_for(std::chrono::milliseconds(tickMs));
        }
        // Game over prompt
        gameOverScreen();
        if (exitRequested)
            return score;
        // Poll for R (restart) or Q (quit)
        while (true)
        {
#ifdef _WIN32
            bool restart = false;
            bool quit = false;
            if (g_hIn != INVALID_HANDLE_VALUE)
            {
                INPUT_RECORD records[16];
                DWORD count = 0;
                if (PeekConsoleInput(g_hIn, records, 16, &count) && count > 0)
                {
                    if (ReadConsoleInput(g_hIn, records, count, &count))
                    {
                        for (DWORD i = 0; i < count; ++i)
                        {
                            const INPUT_RECORD &rec = records[i];
                            if (rec.EventType == KEY_EVENT && rec.Event.KeyEvent.bKeyDown)
                            {
                                WORD vk = rec.Event.KeyEvent.wVirtualKeyCode;
                                if (vk == 'R')
                                    restart = true;
                                if (vk == 'Q' || vk == 'X')
                                {
                                    quit = true;
                                    if (vk == 'X')
                                        exitRequested = true;
                                }
                            }
                        }
                    }
                }
            }
            // Fallback polling for restart/quit
            if (!restart && !quit)
            {
                auto isKey = [](int vk)
                { return (GetAsyncKeyState(vk) & 0x8000) != 0; };
                if (isKey('R'))
                    restart = true;
                if (isKey('Q') || isKey('X'))
                {
                    quit = true;
                    if (isKey('X'))
                        exitRequested = true;
                }
            }
#else
            bool restart = false;
            bool quit = false;
            int ch;
            while ((ch = ::getchar()) != EOF)
            {
                if (ch == 'r' || ch == 'R')
                    restart = true;
                if (ch == 'q' || ch == 'Q' || ch == 'x' || ch == 'X')
                {
                    quit = true;
                    if (ch == 'x' || ch == 'X')
                        exitRequested = true;
                }
            }
#endif
            if (restart)
                break;
            if (quit)
                return score;
            std::this_thread::sleep_for(std::chrono::milliseconds(30));
        }
    }
}

void Game::chooseDifficulty()
{
    std::cout << "\nSET DIFFICULTY\n1: Easy\n2: Medium\n3: Hard\nChoice: ";
#ifndef _WIN32
    // Temporarily restore cooked mode in WSL/POSIX to take interactive input
    if (g_termConfigured)
    {
        tcsetattr(STDIN_FILENO, TCSANOW, &g_origTerm);
        if (g_origFlags != -1)
        {
            fcntl(STDIN_FILENO, F_SETFL, g_origFlags);
        }
    }
#endif
    int choice = 0;
    if (!(std::cin >> choice))
    {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        choice = 2;
    }
#ifndef _WIN32
    // Re-apply raw, non-blocking mode for gameplay
    {
        termios raw = g_origTerm;
        raw.c_lflag &= ~(ICANON | ECHO);
        raw.c_cc[VMIN] = 0;
        raw.c_cc[VTIME] = 0;
        tcsetattr(STDIN_FILENO, TCSANOW, &raw);
        int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
        if (flags != -1)
        {
            fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
        }
        g_termConfigured = true;
    }
#endif
    switch (choice)
    {
    case 1:
        tickMs = 150;
        break;
    case 2:
        tickMs = 100;
        break;
    case 3:
        tickMs = 50;
        break;
    default:
        tickMs = 100;
        break;
    }
}
