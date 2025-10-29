// Snake game using Notcurses for rendering and POSIX (termios) input
#include "game.h"
#include <fstream>
#include <chrono>
#include <thread>
#include <algorithm>
#include <clocale>
#include <cstdint>

// Linux/WSL: Notcurses
#include <notcurses/notcurses.h>
#include <unistd.h>

// ----------------------
// Helpers (platform-agnostic)
// ----------------------
namespace
{
    constexpr char WALL_CHAR = '#';
    constexpr char SNAKE_HEAD_CHAR = '@';
    constexpr char SNAKE_BODY_CHAR = 'O';
    constexpr char FRUIT_CHAR = '*';
}

// ----------------------
// Game lifecycle
// ----------------------
Game::Game(int width, int height, const std::string &name)
    : width(width), height(height),
      snake(width / 2, height / 2, 3),
      fruit(width, height),
      playerName(name)
{
    loadHighScore();
    chooseDifficulty();
    // Ensure fruit not on snake at start
    fruit.respawn([&](const Point &p)
                  { return snake.contains(p); });
}

Game::~Game()
{
    saveHighScore();
}

// -------------- Globals & helpers --------------
namespace
{
    // Keep Notcurses handles accessible to const render/gameOver functions
    static notcurses *g_nc = nullptr;
    static ncplane *g_stdp = nullptr;
    inline void set_fg(ncplane *n, uint8_t r, uint8_t g, uint8_t b) { ncplane_set_fg_rgb8(n, r, g, b); }
    inline void set_bg_default(ncplane *n) { ncplane_set_bg_default(n); }
    inline void set_style(ncplane *n, uint16_t s) { ncplane_set_styles(n, s); }
    inline void clear_style(ncplane *n) { ncplane_set_styles(n, 0); }
}

int Game::run()
{
    // Initialize Notcurses
    setlocale(LC_ALL, "");
    notcurses_options opts{}; // defaults
    struct notcurses *nc = notcurses_init(&opts, nullptr);
    if (!nc)
    {
        return 1;
    }
    ncplane *stdp = notcurses_stdplane(nc);
    // store for later renders
    g_nc = nc;
    g_stdp = stdp;

    // Make sure our target area fits in the terminal
    unsigned termh = 0, termw = 0;
    ncplane_dim_yx(stdp, &termh, &termw);
    if (height > termh || width > termw)
    {
        ncplane_putstr_yx(stdp, 0, 0, "Terminal too small for configured game size.");
        ncplane_putstr_yx(stdp, 1, 0, "Resize terminal or adjust width/height.");
        notcurses_render(nc);
        notcurses_stop(nc);
        return 1;
    }

    auto lastTick = std::chrono::steady_clock::now();

    while (!exitRequested)
    {
        // Input
        processInput();

        // Tick based on tickMs when not paused and not over
        if (!paused && !over)
        {
            auto now = std::chrono::steady_clock::now();
            if (std::chrono::duration_cast<std::chrono::milliseconds>(now - lastTick).count() >= tickMs)
            {
                lastTick = now;
                update();
            }
        }

        // Render
        render();
        notcurses_render(nc);

        // Small sleep to avoid busy loop
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }

    notcurses_stop(nc);
    return score;
}

void Game::processInput()
{
    if (!g_nc)
        return;
    ncinput ni{};
    // Drain all pending inputs non-blocking
    while (true)
    {
        uint32_t key = notcurses_get_nblock(g_nc, &ni);
        if (key == 0u)
            break; // no input available
        if (key == (uint32_t)-1)
            break; // error

        // If a modal dialog is open, navigate/select options
        if (dialogOpen)
        {
            int maxIdx = (dialogType == DialogType::Pause ? 2 : 1);
            if (key == NCKEY_UP || key == NCKEY_LEFT)
            {
                dialogIndex = (dialogIndex - 1 + (maxIdx + 1)) % (maxIdx + 1);
                continue;
            }
            if (key == NCKEY_DOWN || key == NCKEY_RIGHT)
            {
                dialogIndex = (dialogIndex + 1) % (maxIdx + 1);
                continue;
            }
            if (key == 'q' || key == 'Q')
            {
                exitRequested = true;
                continue;
            }
            if (key == ' ' || key == '\n' || key == NCKEY_ENTER)
            {
                if (dialogType == DialogType::Pause)
                {
                    if (dialogIndex == 0)
                    {
                        paused = false;
                        closeDialog();
                    }
                    else if (dialogIndex == 1)
                    {
                        reset();
                        paused = false;
                        closeDialog();
                    }
                    else if (dialogIndex == 2)
                    {
                        exitRequested = true;
                    }
                }
                else if (dialogType == DialogType::GameOver)
                {
                    if (dialogIndex == 0)
                    {
                        reset();
                        closeDialog();
                    }
                    else if (dialogIndex == 1)
                    {
                        exitRequested = true;
                    }
                }
                continue;
            }
            if (key == 'p' || key == 'P')
            {
                if (dialogType == DialogType::Pause)
                {
                    paused = false;
                    closeDialog();
                }
                continue;
            }
            // ignore other keys while dialog is open
            continue;
        }

        auto handle_dir = [&](Direction d)
        { if (!paused && !over) snake.setDirection(d); };

        if (key == NCKEY_UP)
        {
            handle_dir(Direction::Up);
        }
        else if (key == NCKEY_DOWN)
        {
            handle_dir(Direction::Down);
        }
        else if (key == NCKEY_LEFT)
        {
            handle_dir(Direction::Left);
        }
        else if (key == NCKEY_RIGHT)
        {
            handle_dir(Direction::Right);
        }
        else if (key == 'w' || key == 'W')
        {
            handle_dir(Direction::Up);
        }
        else if (key == 's' || key == 'S')
        {
            handle_dir(Direction::Down);
        }
        else if (key == 'a' || key == 'A')
        {
            handle_dir(Direction::Left);
        }
        else if (key == 'd' || key == 'D')
        {
            handle_dir(Direction::Right);
        }
        else if (key == 'q' || key == 'Q')
        {
            exitRequested = true;
        }
        else if (key == ' ' || key == 'p' || key == 'P')
        {
            paused = !paused;
            if (paused)
            {
                openDialog(DialogType::Pause);
                dialogIndex = 0;
            }
            else
            {
                closeDialog();
            }
        }
        else if (key == 'r' || key == 'R' || key == 'c' || key == 'C')
        {
            if (over)
                reset();
        }
        else if (key == 'g' || key == 'G')
        {
            // cycle snake glyph style
            switch (snakeStyle)
            {
            case SnakeGlyphStyle::Light:
                snakeStyle = SnakeGlyphStyle::Heavy;
                break;
            case SnakeGlyphStyle::Heavy:
                snakeStyle = SnakeGlyphStyle::Rounded;
                break;
            case SnakeGlyphStyle::Rounded:
                snakeStyle = SnakeGlyphStyle::Scales;
                break;
            case SnakeGlyphStyle::Scales:
                snakeStyle = SnakeGlyphStyle::DoubleLine;
                break;
            case SnakeGlyphStyle::DoubleLine:
                snakeStyle = SnakeGlyphStyle::Block;
                break;
            case SnakeGlyphStyle::Block:
                snakeStyle = SnakeGlyphStyle::Arrow;
                break;
            case SnakeGlyphStyle::Arrow:
                snakeStyle = SnakeGlyphStyle::Dotted;
                break;
            case SnakeGlyphStyle::Dotted:
                snakeStyle = SnakeGlyphStyle::Braille;
                break;
            case SnakeGlyphStyle::Braille:
                snakeStyle = SnakeGlyphStyle::Light;
                break;
            }
        }
    }
}

void Game::render() const
{
    if (!g_stdp)
        return;
    ncplane_erase(g_stdp);
    // Compute centered origin for board and HUD
    unsigned ph = 0, pw = 0;
    ncplane_dim_yx(g_stdp, &ph, &pw);
    const int HUDW = 24; // fixed side panel width
    int oy = (int)ph / 2 - height / 2;
    if (oy < 0)
        oy = 0;
    int ox = (int)pw / 2 - (width + HUDW + 1) / 2;
    if (ox < 0)
        ox = 0;
    int hx = ox + width + 1; // HUD x

    // Draw board border with UTF-8 double lines and gradient color
    const char *hline = "═";
    const char *vline = "║";
    const char *tl = "╔";
    const char *tr = "╗";
    const char *bl = "╚";
    const char *br = "╝";
    auto grad = [&](float t, uint8_t &r, uint8_t &g, uint8_t &b)
    {
        // from bluish (120,160,255) to aqua (120,255,200)
        int r1 = 120, g1 = 160, b1 = 255, r2 = 120, g2 = 255, b2 = 200;
        r = (uint8_t)(r1 + (r2 - r1) * t);
        g = (uint8_t)(g1 + (g2 - g1) * t);
        b = (uint8_t)(b1 + (b2 - b1) * t);
    };
    // corners
    uint8_t cr, cg, cb;
    grad(0.0f, cr, cg, cb);
    set_fg(g_stdp, cr, cg, cb);
    ncplane_putstr_yx(g_stdp, oy + 0, ox + 0, tl);
    grad(1.0f, cr, cg, cb);
    set_fg(g_stdp, cr, cg, cb);
    ncplane_putstr_yx(g_stdp, oy + 0, ox + width - 1, tr);
    grad(0.0f, cr, cg, cb);
    set_fg(g_stdp, cr, cg, cb);
    ncplane_putstr_yx(g_stdp, oy + height - 1, ox + 0, bl);
    grad(1.0f, cr, cg, cb);
    set_fg(g_stdp, cr, cg, cb);
    ncplane_putstr_yx(g_stdp, oy + height - 1, ox + width - 1, br);
    // top/bottom
    for (int x = 1; x < width - 1; ++x)
    {
        float t = (float)x / (float)(width - 1);
        grad(t, cr, cg, cb);
        set_fg(g_stdp, cr, cg, cb);
        ncplane_putstr_yx(g_stdp, oy + 0, ox + x, hline);
        ncplane_putstr_yx(g_stdp, oy + height - 1, ox + x, hline);
    }
    // sides
    for (int y = 1; y < height - 1; ++y)
    {
        float t = (float)y / (float)(height - 1);
        grad(t, cr, cg, cb);
        set_fg(g_stdp, cr, cg, cb);
        ncplane_putstr_yx(g_stdp, oy + y, ox + 0, vline);
        grad(1.0f - t, cr, cg, cb);
        set_fg(g_stdp, cr, cg, cb);
        ncplane_putstr_yx(g_stdp, oy + y, ox + width - 1, vline);
    }

    // Side HUD panel
    set_fg(g_stdp, 200, 230, 255);
    ncplane_putstr_yx(g_stdp, oy + 0, hx + 0, "┌");
    for (int x = 1; x < HUDW - 1; ++x)
        ncplane_putstr_yx(g_stdp, oy + 0, hx + x, "─");
    ncplane_putstr_yx(g_stdp, oy + 0, hx + HUDW - 1, "┐");
    for (int y = 1; y < height - 1; ++y)
    {
        ncplane_putstr_yx(g_stdp, oy + y, hx + 0, "│");
        ncplane_putstr_yx(g_stdp, oy + y, hx + HUDW - 1, "│");
    }
    ncplane_putstr_yx(g_stdp, oy + height - 1, hx + 0, "└");
    for (int x = 1; x < HUDW - 1; ++x)
        ncplane_putstr_yx(g_stdp, oy + height - 1, hx + x, "─");
    ncplane_putstr_yx(g_stdp, oy + height - 1, hx + HUDW - 1, "┘");
    // HUD content
    set_fg(g_stdp, 120, 200, 255);
    ncplane_putstr_yx(g_stdp, oy + 1, hx + 2, "Player:");
    set_fg(g_stdp, 255, 255, 255);
    ncplane_putstr_yx(g_stdp, oy + 1, hx + 10, playerName.c_str());
    set_fg(g_stdp, 255, 215, 0);
    ncplane_putstr_yx(g_stdp, oy + 3, hx + 2, "Score:");
    set_fg(g_stdp, 255, 255, 255);
    ncplane_putstr_yx(g_stdp, oy + 3, hx + 10, std::to_string(score).c_str());
    set_fg(g_stdp, 0, 255, 180);
    ncplane_putstr_yx(g_stdp, oy + 5, hx + 2, "High:");
    set_fg(g_stdp, 255, 255, 255);
    ncplane_putstr_yx(g_stdp, oy + 5, hx + 10, std::to_string(highScore).c_str());
    set_fg(g_stdp, 200, 200, 200);
    ncplane_putstr_yx(g_stdp, oy + 7, hx + 2, "Controls:");
    set_fg(g_stdp, 180, 180, 180);
    ncplane_putstr_yx(g_stdp, oy + 8, hx + 2, "Arrows/WASD move");
    ncplane_putstr_yx(g_stdp, oy + 9, hx + 2, "p/space pause");
    ncplane_putstr_yx(g_stdp, oy + 10, hx + 2, "q quit");
    // Optional hint for glyph styles
    set_fg(g_stdp, 170, 170, 170);
    ncplane_putstr_yx(g_stdp, oy + 12, hx + 2, "g: change snake style");

    // Fruit (solid circle)
    const auto &fp = fruit.position();
    set_fg(g_stdp, 255, 80, 80);
    ncplane_putstr_yx(g_stdp, oy + fp.y, ox + fp.x, "●");

    // Snake with connected glyphs and directional head + multi-stop gradient
    const auto &segs = snake.segments();
    int nseg = (int)segs.size();
    // Color gradient: lime (head) -> yellow (mid) -> cyan (tail)
    auto sgrad = [&](float t, uint8_t &r, uint8_t &g, uint8_t &b)
    {
        int r0 = 80, g0 = 255, b0 = 120; // lime
        int r1 = 255, g1 = 220, b1 = 0;  // yellow
        int r2 = 0, g2 = 220, b2 = 255;  // cyan
        if (t <= 0.5f)
        {
            float u = t * 2.0f;
            r = (uint8_t)(r0 + (int)((r1 - r0) * u));
            g = (uint8_t)(g0 + (int)((g1 - g0) * u));
            b = (uint8_t)(b0 + (int)((b1 - b0) * u));
        }
        else
        {
            float u = (t - 0.5f) * 2.0f;
            r = (uint8_t)(r1 + (int)((r2 - r1) * u));
            g = (uint8_t)(g1 + (int)((g2 - g1) * u));
            b = (uint8_t)(b1 + (int)((b2 - b1) * u));
        }
    };
    for (int i = 0; i < nseg; ++i)
    {
        const Point &c = segs[i];
        float t = (float)i / std::max(1, nseg - 1); // 0=head .. 1=tail
        uint8_t r, g, b;
        sgrad(t, r, g, b);
        set_fg(g_stdp, r, g, b);
        const char *glyph = "■";
        if (i == 0)
        {
            // head based on current direction
            switch (snake.getDirection())
            {
            case Direction::Up:
                glyph = "▲";
                break;
            case Direction::Down:
                glyph = "▼";
                break;
            case Direction::Left:
                glyph = "◀";
                break;
            case Direction::Right:
                glyph = "▶";
                break;
            }
        }
        else if (i == nseg - 1)
        {
            // tail dot
            glyph = "•";
        }
        else
        {
            const Point &p = segs[i - 1];
            const Point &n = segs[i + 1];
            bool up = (p.y < c.y) || (n.y < c.y);
            bool down = (p.y > c.y) || (n.y > c.y);
            bool left = (p.x < c.x) || (n.x < c.x);
            bool right = (p.x > c.x) || (n.x > c.x);
            switch (snakeStyle)
            {
            case SnakeGlyphStyle::Light:
                if ((left && right) && !(up || down))
                    glyph = "─";
                else if ((up && down) && !(left || right))
                    glyph = "│";
                else if ((left && up))
                    glyph = "┘"; // connects left+up
                else if ((left && down))
                    glyph = "┐"; // connects left+down
                else if ((right && up))
                    glyph = "└"; // connects right+up
                else if ((right && down))
                    glyph = "┌"; // connects right+down
                else
                    glyph = "■";
                break;
            case SnakeGlyphStyle::Heavy:
                if ((left && right) && !(up || down))
                    glyph = "━";
                else if ((up && down) && !(left || right))
                    glyph = "┃";
                else if ((left && up))
                    glyph = "┛";
                else if ((left && down))
                    glyph = "┓";
                else if ((right && up))
                    glyph = "┗";
                else if ((right && down))
                    glyph = "┏";
                else
                    glyph = "■";
                break;
            case SnakeGlyphStyle::Rounded:
                if ((left && right) && !(up || down))
                    glyph = "─";
                else if ((up && down) && !(left || right))
                    glyph = "│";
                else if ((left && up))
                    glyph = "╯";
                else if ((left && down))
                    glyph = "╮";
                else if ((right && up))
                    glyph = "╰";
                else if ((right && down))
                    glyph = "╭";
                else
                    glyph = "■";
                break;
            case SnakeGlyphStyle::Scales:
                // ignore connectivity; use a scale tile pattern
                glyph = ((i & 1) == 0) ? "▚" : "▞";
                break;
            case SnakeGlyphStyle::DoubleLine:
                if ((left && right) && !(up || down))
                    glyph = "═";
                else if ((up && down) && !(left || right))
                    glyph = "║";
                else if ((left && up))
                    glyph = "╝";
                else if ((left && down))
                    glyph = "╗";
                else if ((right && up))
                    glyph = "╚";
                else if ((right && down))
                    glyph = "╔";
                else
                    glyph = "■";
                break;
            case SnakeGlyphStyle::Block:
                glyph = "█";
                break;
            case SnakeGlyphStyle::Arrow:
            {
                int dx = n.x - c.x;
                int dy = n.y - c.y;
                if (dx > 0)
                    glyph = "▷";
                else if (dx < 0)
                    glyph = "◁";
                else if (dy > 0)
                    glyph = "▽";
                else if (dy < 0)
                    glyph = "△";
                else
                    glyph = "■";
                break;
            }
            case SnakeGlyphStyle::Dotted:
                glyph = (i % 3 == 0) ? "●" : (i % 3 == 1 ? "•" : "·");
                break;
            case SnakeGlyphStyle::Braille:
            {
                static const char *pat[] = {"⣿", "⣾", "⣷", "⣯", "⣟"};
                glyph = pat[i % 5];
                break;
            }
            }
        }
        ncplane_putstr_yx(g_stdp, oy + c.y, ox + c.x, glyph);
    }

    // Modal dialog (Pause or GameOver)
    if (dialogOpen)
    {
        int drows = 7;
        int dcols = 32;
        int dy = oy + height / 2 - drows / 2;
        int dx = ox + width / 2 - dcols / 2;
        if (dy < 1)
            dy = 1;
        if (dx < 1)
            dx = 1;
        set_fg(g_stdp, 255, 255, 255);
        // Top border
        ncplane_putstr_yx(g_stdp, dy + 0, dx + 0, "╔");
        for (int x = 1; x < dcols - 1; ++x)
            ncplane_putstr_yx(g_stdp, dy + 0, dx + x, "═");
        ncplane_putstr_yx(g_stdp, dy + 0, dx + dcols - 1, "╗");
        // Sides
        for (int y = 1; y < drows - 1; ++y)
        {
            ncplane_putstr_yx(g_stdp, dy + y, dx + 0, "║");
            ncplane_putstr_yx(g_stdp, dy + y, dx + dcols - 1, "║");
        }
        // Bottom
        ncplane_putstr_yx(g_stdp, dy + drows - 1, dx + 0, "╚");
        for (int x = 1; x < dcols - 1; ++x)
            ncplane_putstr_yx(g_stdp, dy + drows - 1, dx + x, "═");
        ncplane_putstr_yx(g_stdp, dy + drows - 1, dx + dcols - 1, "╝");

        std::string title = (dialogType == DialogType::Pause) ? "Pause" : "Game Over";
        int tx = dx + (dcols - (int)title.size()) / 2;
        set_fg(g_stdp, 120, 200, 255);
        ncplane_putstr_yx(g_stdp, dy + 1, tx, title.c_str());

        auto draw_option = [&](int row, int idx, const char *label)
        {
            bool sel = (dialogIndex == idx);
            if (sel)
                set_fg(g_stdp, 255, 255, 255);
            else
                set_fg(g_stdp, 180, 180, 180);
            std::string line = sel ? (std::string("▶ ") + label + " ◀") : (std::string("  ") + label);
            ncplane_putstr_yx(g_stdp, dy + row, dx + 3, line.c_str());
        };
        if (dialogType == DialogType::Pause)
        {
            draw_option(3, 0, "Resume");
            draw_option(4, 1, "Restart");
            draw_option(5, 2, "Quit");
        }
        else
        {
            draw_option(3, 0, "Restart");
            draw_option(4, 1, "Quit");
        }
    }
}

void Game::update()
{
    // Compute next head and collisions
    Point next = snake.nextHead();

    // Walls
    if (next.x <= 0 || next.x >= width - 1 || next.y <= 0 || next.y >= height - 1)
    {
        over = true;
        openDialog(DialogType::GameOver);
        return;
    }

    // Self
    if (snake.hitsSelf(next))
    {
        over = true;
        openDialog(DialogType::GameOver);
        return;
    }

    bool grow = false;
    if (next == fruit.position())
    {
        grow = true;
        score += 10;
        if (score > highScore)
            highScore = score;
        fruit.respawn([&](const Point &p)
                      { return snake.contains(p) || p == next; });
    }

    snake.move(grow);
}

void Game::gameOverScreen() const
{
    // Legacy blocking screen; unused in new non-blocking loop
}

void Game::reset()
{
    score = 0;
    over = false;
    exitRequested = false;
    dialogOpen = false;
    dialogType = DialogType::None;
    dialogIndex = 0;
    paused = false;
    snake = Snake(width / 2, height / 2, 3);
    fruit = Fruit(width, height);
    fruit.respawn([&](const Point &p)
                  { return snake.contains(p); });
}

void Game::chooseDifficulty()
{
    // Simple preset based on name or default; could be interactive later
    // Keep it non-interactive to avoid extra UI complexity in the TUI
    tickMs = 120; // default normal
}

void Game::loadHighScore()
{
    std::ifstream in(highScoreFile);
    if (!in.good())
        return;
    int hs = 0;
    in >> hs;
    if (in)
        highScore = hs;
}

void Game::saveHighScore()
{
    std::ofstream out(highScoreFile, std::ios::trunc);
    if (!out.good())
        return;
    out << highScore << "\n";
}

void Game::openDialog(DialogType t)
{
    dialogType = t;
    dialogOpen = true;
    dialogIndex = 0;
}

void Game::closeDialog()
{
    dialogOpen = false;
    dialogType = DialogType::None;
    dialogIndex = 0;
}
