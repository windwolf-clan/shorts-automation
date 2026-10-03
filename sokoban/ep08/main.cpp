#include <windows.h>
#include <vector>

const int TILE = 80;
const int MAP_W = 9;
const int MAP_H = 6;

const char* LEVEL[MAP_H] = {
    "#########",
    "#  .#   #",
    "# # .   #",
    "# $ .$$ #",
    "## ##@ ##",
    "#########",
};

const COLORREF FLOOR_COLOR       = RGB(222, 214, 196);
const COLORREF FLOOR_LINE_COLOR  = RGB(205, 196, 176);
const COLORREF WALL_COLOR        = RGB(70, 78, 102);
const COLORREF WALL_TOP_COLOR    = RGB(98, 108, 138);
const COLORREF GOAL_COLOR        = RGB(214, 90, 80);
const COLORREF BOX_COLOR         = RGB(196, 142, 72);
const COLORREF BOX_EDGE_COLOR    = RGB(120, 80, 36);
const COLORREF BOX_OK_COLOR      = RGB(92, 170, 110);
const COLORREF BOX_OK_EDGE_COLOR = RGB(48, 110, 64);
const COLORREF CLEAR_TEXT_COLOR  = RGB(255, 255, 255);
const COLORREF CLEAR_BACK_COLOR  = RGB(48, 142, 88);
const COLORREF PLAYER_COLOR      = RGB(64, 128, 220);
const COLORREF PLAYER_EDGE_COLOR = RGB(30, 70, 150);

char g_tile[MAP_H][MAP_W];  // '#' 벽, ' ' 바닥, '.' 목표
bool g_box[MAP_H][MAP_W];
int g_playerX = 0;
int g_playerY = 0;

const int STEP_MS = 120;   // 한 칸 이동에 걸리는 시간

struct Slide
{
    int fromX, fromY;
    int toX, toY;
    DWORD start;
    bool active;
};

Slide g_playerSlide;
Slide g_boxSlide;

void LoadLevel()
{
    for (int y = 0; y < MAP_H; y++)
    {
        for (int x = 0; x < MAP_W; x++)
        {
            char c = LEVEL[y][x];
            g_tile[y][x] = (c == '#' || c == '.') ? c : ' ';
            g_box[y][x] = (c == '$');
            if (c == '@')
            {
                g_playerX = x;
                g_playerY = y;
            }
        }
    }
}

void StartSlide(Slide& s, int fx, int fy, int tx, int ty)
{
    s.fromX = fx; s.fromY = fy;
    s.toX = tx;   s.toY = ty;
    s.start = GetTickCount();
    s.active = true;
}

RECT SlideRect(const Slide& s)
{
    DWORD past = GetTickCount() - s.start;
    float t = past >= STEP_MS ? 1.0f : (float)past / STEP_MS;
    float x = s.fromX + (s.toX - s.fromX) * t;
    float y = s.fromY + (s.toY - s.fromY) * t;
    RECT r = { (int)(x * TILE), (int)(y * TILE), 0, 0 };
    r.right = r.left + TILE;
    r.bottom = r.top + TILE;
    return r;
}

void UpdateSlides()
{
    DWORD now = GetTickCount();
    if (g_playerSlide.active && now - g_playerSlide.start >= STEP_MS)
        g_playerSlide.active = false;
    if (g_boxSlide.active && now - g_boxSlide.start >= STEP_MS)
        g_boxSlide.active = false;
}

void FillColor(HDC hdc, RECT r, COLORREF color)
{
    HBRUSH brush = CreateSolidBrush(color);
    FillRect(hdc, &r, brush);
    DeleteObject(brush);
}

void FrameColor(HDC hdc, RECT r, COLORREF color)
{
    HBRUSH brush = CreateSolidBrush(color);
    FrameRect(hdc, &r, brush);
    DeleteObject(brush);
}

void DrawShape(HDC hdc, RECT r, COLORREF fill, COLORREF edge, bool circle)
{
    HBRUSH brush = CreateSolidBrush(fill);
    HPEN pen = CreatePen(PS_SOLID, 4, edge);
    HGDIOBJ oldBrush = SelectObject(hdc, brush);
    HGDIOBJ oldPen = SelectObject(hdc, pen);

    if (circle)
        Ellipse(hdc, r.left, r.top, r.right, r.bottom);
    else
        Rectangle(hdc, r.left, r.top, r.right, r.bottom);

    SelectObject(hdc, oldBrush);
    SelectObject(hdc, oldPen);
    DeleteObject(brush);
    DeleteObject(pen);
}

RECT TileRect(int x, int y)
{
    return { x * TILE, y * TILE, (x + 1) * TILE, (y + 1) * TILE };
}

void DrawTile(HDC hdc, int x, int y)
{
    RECT r = TileRect(x, y);
    if (g_tile[y][x] == '#')
    {
        RECT top = { r.left, r.top, r.right, r.top + 12 };
        FillColor(hdc, r, WALL_COLOR);
        FillColor(hdc, top, WALL_TOP_COLOR);
        return;
    }

    FillColor(hdc, r, FLOOR_COLOR);
    FrameColor(hdc, r, FLOOR_LINE_COLOR);
    if (g_tile[y][x] == '.')
    {
        RECT dot = r;
        InflateRect(&dot, -28, -28);
        DrawShape(hdc, dot, GOAL_COLOR, GOAL_COLOR, true);
    }
}

void DrawBoxRect(HDC hdc, RECT box, bool onGoal)
{
    InflateRect(&box, -10, -10);
    if (onGoal)
        DrawShape(hdc, box, BOX_OK_COLOR, BOX_OK_EDGE_COLOR, false);
    else
        DrawShape(hdc, box, BOX_COLOR, BOX_EDGE_COLOR, false);
}

void DrawBox(HDC hdc, int x, int y)
{
    DrawBoxRect(hdc, TileRect(x, y), g_tile[y][x] == '.');
}

void DrawClear(HDC hdc)
{
    int mid = MAP_H * TILE / 2;
    RECT banner = { 0, mid - 60, MAP_W * TILE, mid + 60 };
    FillColor(hdc, banner, CLEAR_BACK_COLOR);

    HFONT font = CreateFontW(64, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                             DEFAULT_CHARSET, 0, 0, 0, 0, L"Segoe UI");
    HGDIOBJ oldFont = SelectObject(hdc, font);
    SetTextColor(hdc, CLEAR_TEXT_COLOR);
    SetBkMode(hdc, TRANSPARENT);
    DrawTextW(hdc, L"CLEAR!  (R: 다시, Z: 되돌리기)", -1, &banner,
              DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    SelectObject(hdc, oldFont);
    DeleteObject(font);
}

void DrawMap(HDC hdc)
{
    for (int y = 0; y < MAP_H; y++)
    {
        for (int x = 0; x < MAP_W; x++)
        {
            DrawTile(hdc, x, y);
            bool sliding = g_boxSlide.active &&
                           x == g_boxSlide.toX && y == g_boxSlide.toY;
            if (g_box[y][x] && !sliding)
                DrawBox(hdc, x, y);
        }
    }

    if (g_boxSlide.active)
        DrawBoxRect(hdc, SlideRect(g_boxSlide), false);
}

void DrawPlayer(HDC hdc)
{
    RECT player = g_playerSlide.active ? SlideRect(g_playerSlide)
                                       : TileRect(g_playerX, g_playerY);
    InflateRect(&player, -14, -14);
    DrawShape(hdc, player, PLAYER_COLOR, PLAYER_EDGE_COLOR, true);
}

struct Move
{
    int dx;
    int dy;
    bool pushed;   // 이 이동에서 상자를 밀었는가
};

std::vector<Move> g_history;

bool IsWall(int x, int y)
{
    return g_tile[y][x] == '#';
}

bool IsCleared()
{
    for (int y = 0; y < MAP_H; y++)
        for (int x = 0; x < MAP_W; x++)
            if (g_box[y][x] && g_tile[y][x] != '.')
                return false;
    return true;
}

void TryMove(int dx, int dy)
{
    if (g_playerSlide.active)
        return;

    int nx = g_playerX + dx;
    int ny = g_playerY + dy;
    if (IsWall(nx, ny))
        return;

    bool pushed = false;
    if (g_box[ny][nx])
    {
        int bx = nx + dx;
        int by = ny + dy;
        if (IsWall(bx, by) || g_box[by][bx])
            return;

        g_box[ny][nx] = false;
        g_box[by][bx] = true;
        StartSlide(g_boxSlide, nx, ny, bx, by);
        pushed = true;
    }

    StartSlide(g_playerSlide, g_playerX, g_playerY, nx, ny);
    g_playerX = nx;
    g_playerY = ny;
    g_history.push_back({ dx, dy, pushed });
}

void Undo()
{
    if (g_history.empty())
        return;

    Move m = g_history.back();
    g_history.pop_back();

    if (m.pushed)
    {
        g_box[g_playerY + m.dy][g_playerX + m.dx] = false;
        g_box[g_playerY][g_playerX] = true;
    }

    g_playerX -= m.dx;
    g_playerY -= m.dy;
}

void PollInput()
{
    if (GetAsyncKeyState(VK_LEFT) & 0x8000)  TryMove(-1, 0);
    if (GetAsyncKeyState(VK_RIGHT) & 0x8000) TryMove(1, 0);
    if (GetAsyncKeyState(VK_UP) & 0x8000)    TryMove(0, -1);
    if (GetAsyncKeyState(VK_DOWN) & 0x8000)  TryMove(0, 1);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_KEYDOWN:
        switch (wParam)
        {
        case 'R':       LoadLevel(); g_history.clear(); break;
        case 'Z':       Undo(); break;
        case VK_ESCAPE: DestroyWindow(hWnd); return 0;
        }
        InvalidateRect(hWnd, nullptr, TRUE);
        return 0;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);

        DrawMap(hdc);
        DrawPlayer(hdc);
        if (IsCleared())
            DrawClear(hdc);
        EndPaint(hWnd, &ps);
        return 0;
    }

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hWnd, msg, wParam, lParam);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    PWSTR pCmdLine, int nCmdShow)
{
    LoadLevel();

    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = L"SokobanWindow";
    RegisterClassExW(&wc);

    RECT rc = { 0, 0, MAP_W * TILE, MAP_H * TILE };
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
    int width = rc.right - rc.left;
    int height = rc.bottom - rc.top;

    HWND hWnd = CreateWindowExW(0, L"SokobanWindow", L"Sokoban",
                                WS_OVERLAPPEDWINDOW,
                                CW_USEDEFAULT, CW_USEDEFAULT, width, height,
                                nullptr, nullptr, hInstance, nullptr);
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    MSG msg = {};
    while (msg.message != WM_QUIT)
    {
        if (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
            continue;
        }

        PollInput();
        UpdateSlides();
        InvalidateRect(hWnd, nullptr, TRUE);
        UpdateWindow(hWnd);
        Sleep(1);
    }
    return (int)msg.wParam;
}
