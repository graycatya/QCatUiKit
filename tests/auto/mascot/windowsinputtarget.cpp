// SPDX-License-Identifier: Apache-2.0
#define UNICODE
#define _UNICODE
#include <windows.h>

namespace
{
LRESULT CALLBACK targetProc(HWND window, UINT message, WPARAM wp, LPARAM lp)
{
    if (message == WM_LBUTTONDOWN || message == WM_KEYDOWN)
    {
        const auto name = message == WM_LBUTTONDOWN ? L"QCatClicks" : L"QCatKeys";
        const auto count = reinterpret_cast<UINT_PTR>(GetPropW(window, name));
        SetPropW(window, name, reinterpret_cast<HANDLE>(count + 1));
        return 0;
    }
    if (message == WM_PAINT)
    {
        PAINTSTRUCT paint{};
        const auto dc = BeginPaint(window, &paint);
        RECT rect{};
        GetClientRect(window, &rect);
        const auto brush = CreateSolidBrush(RGB(29, 71, 99));
        FillRect(dc, &rect, brush);
        DeleteObject(brush);
        EndPaint(window, &paint);
        return 0;
    }
    if (message == WM_DESTROY)
    {
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(window, message, wp, lp);
}
} // namespace
int main(int argc, char **argv)
{
    if (argc != 2)
        return 2;
    wchar_t title[128]{};
    if (!MultiByteToWideChar(CP_UTF8, 0, argv[1], -1, title, 128))
        return 2;
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    WNDCLASSW wc{};
    wc.lpfnWndProc = targetProc;
    wc.hInstance = GetModuleHandleW(nullptr);
    wc.lpszClassName = L"QCatMascotInputTarget";
    if (!RegisterClassW(&wc))
        return 3;
    const auto window = CreateWindowExW(0, wc.lpszClassName, title, WS_POPUP, 100, 100, 1000, 800,
                                        nullptr, nullptr, wc.hInstance, nullptr);
    if (!window)
        return 4;
    ShowWindow(window, SW_SHOWNOACTIVATE);
    SetForegroundWindow(window);
    MSG message{};
    while (GetMessageW(&message, nullptr, 0, 0) > 0)
    {
        TranslateMessage(&message);
        DispatchMessageW(&message);
    }
    return 0;
}
