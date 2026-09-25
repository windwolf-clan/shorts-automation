#include <windows.h>

const int TILE = 80;

const COLORREF FLOOR_COLOR       = RGB(222, 214, 196);
const COLORREF FLOOR_LINE_COLOR  = RGB(205, 196, 176);
const COLORREF PLAYER_COLOR      = RGB(64, 128, 220);
const COLORREF PLAYER_EDGE_COLOR = RGB(30, 70, 150);

int g_playerX = 0;
int g_playerY = 0;

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

void DrawGrid(HDC hdc, int cols, int rows)
{
    for (int y = 0; y < rows; y++)
    {
        for (int x = 0; x < cols; x++)
        {
            RECT r = TileRect(x, y);
            FillColor(hdc, r, FLOOR_COLOR);
            FrameColor(hdc, r, FLOOR_LINE_COLOR);
        }
    }
}

void DrawPlayer(HDC hdc)
{
    RECT player = TileRect(g_playerX, g_playerY);
    InflateRect(&player, -14, -14);
    DrawShape(hdc, player, PLAYER_COLOR, PLAYER_EDGE_COLOR, true);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_KEYDOWN:
        switch (wParam)
        {
        case VK_LEFT:   g_playerX--; break;
        case VK_RIGHT:  g_playerX++; break;
        case VK_UP:     g_playerY--; break;
        case VK_DOWN:   g_playerY++; break;
        case VK_ESCAPE: DestroyWindow(hWnd); return 0;
        }
        InvalidateRect(hWnd, nullptr, TRUE);
        return 0;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        RECT client;
        GetClientRect(hWnd, &client);
        DrawGrid(hdc, client.right / TILE + 1, client.bottom / TILE + 1);
        DrawPlayer(hdc);
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
    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = L"SokobanWindow";
    RegisterClassExW(&wc);

    HWND hWnd = CreateWindowExW(0, L"SokobanWindow", L"Sokoban",
                                WS_OVERLAPPEDWINDOW,
                                CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
                                nullptr, nullptr, hInstance, nullptr);
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    MSG msg;
    while (GetMessageW(&msg, nullptr, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return (int)msg.wParam;
}
