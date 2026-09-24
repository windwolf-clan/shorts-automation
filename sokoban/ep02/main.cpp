#include <windows.h>
#include <string>

int g_playerX = 0;
int g_playerY = 0;

void UpdateTitle(HWND hWnd)
{
    std::wstring title = L"Sokoban - x: " + std::to_wstring(g_playerX)
                       + L", y: " + std::to_wstring(g_playerY);
    SetWindowTextW(hWnd, title.c_str());
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
        UpdateTitle(hWnd);
        return 0;

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
    UpdateTitle(hWnd);
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
