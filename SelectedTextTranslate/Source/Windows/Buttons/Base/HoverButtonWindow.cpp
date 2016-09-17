#include "Windows\Buttons\Base\HoverButtonWindow.h"

HoverButtonWindow::HoverButtonWindow(HINSTANCE hInstance, RenderingContext* renderingContext, ScrollProvider* scrollProvider, WindowDescriptor descriptor, HWND parentWindow, function<void()> clickCallback)
    : ChildWindow(hInstance, renderingContext, scrollProvider, descriptor, parentWindow)
{
    this->clickCallback = clickCallback;
    this->isHovered = false;
}

void HoverButtonWindow::Initialize()
{
    ChildWindow::Initialize();
    SetWindowLongPtr(hWindow, GWL_WNDPROC, (LONG_PTR)HoverButtonWindow::WndProc);

    RenderStatesDC();
    RenderDC(NULL);
}

LRESULT CALLBACK HoverButtonWindow::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    TRACKMOUSEEVENT tme;
    tme.cbSize = sizeof(TRACKMOUSEEVENT);
    tme.hwndTrack = hWnd;
    tme.dwHoverTime = 10;

    HoverButtonWindow* window = (HoverButtonWindow*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

    switch (message)
    {

    case WM_LBUTTONUP:
        window->clickCallback();
        return TRUE;

    case WM_MOUSEMOVE:
        tme.dwFlags = TME_HOVER | TME_LEAVE;
        TrackMouseEvent(&tme);

        return TRUE;

    case WM_MOUSEHOVER:
        window->isHovered = true;
        window->RenderDC(NULL);
        InvalidateRect(hWnd, NULL, TRUE);

        SetCursor(LoadCursor(NULL, IDC_HAND));

        break;

    case WM_SETCURSOR:
        return TRUE;

    case WM_MOUSELEAVE:
        window->isHovered = false;
        window->RenderDC(NULL);
        InvalidateRect(hWnd, NULL, TRUE);

        SetCursor(LoadCursor(NULL, IDC_ARROW));

        return TRUE;

    default:
        break;
    }

    return ChildWindow::WndProc(hWnd, message, wParam, lParam);
}

SIZE HoverButtonWindow::RenderDC(Renderer* renderer)
{
    HDC sourceDC = isHovered
        ? hoverStateDC
        : normalStateDC;

    DWORD res = renderingContext->CopyDC(sourceDC, inMemoryDC, currentWidth, currentHeight);

    MoveWindow(hWindow, descriptor.X, descriptor.Y, currentWidth, currentHeight, FALSE);

    SIZE size;
    size.cx = currentWidth;
    size.cy = currentHeight;

    return size;
}

HoverButtonWindow::~HoverButtonWindow()
{
    DeleteDC(normalStateDC);
    DeleteDC(hoverStateDC);
}