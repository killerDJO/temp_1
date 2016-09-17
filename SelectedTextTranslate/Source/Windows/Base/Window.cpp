#include "Windows\Base\Window.h"

Window::Window(HINSTANCE hInstance, RenderingContext* renderingContext, ScrollProvider* scrollProvider, WindowDescriptor descriptor)
{
    this->descriptor = descriptor;
    this->hInstance = hInstance;
    this->renderingContext = renderingContext;
    this->scrollProvider = scrollProvider;

    this->currentWidth = descriptor.Width;
    this->currentHeight = descriptor.Height;
}

void Window::Initialize()
{
    WNDCLASSEX windowClass = { 0 };

    if (!GetClassInfoEx(hInstance, className, &windowClass))
    {
        windowClass.hInstance = hInstance;
        windowClass.lpszClassName = className;
        windowClass.cbSize = sizeof(WNDCLASSEX);

        SpecifyWindowClass(&windowClass);

        if (!RegisterClassEx(&windowClass))
        {
            FatalAppExit(0, TEXT("Couldn't register window class!"));
        }
    }
}

void Window::Render(bool preserveVerticalScroll)
{
    int verticalScroll = 0;

    if (preserveVerticalScroll && descriptor.OverflowY == OverflowModes::Scroll)
    {
        verticalScroll = scrollProvider->GetScrollPosition(hWindow, SB_VERT);
    }

    SIZE renderedSize = RenderContent();

    int contentWidth = renderedSize.cx;
    int contentHeight = renderedSize.cy;

    if (descriptor.OverflowX == OverflowModes::Stretch && contentWidth > descriptor.Width)
    {
        currentWidth = contentWidth;
    }

    if (descriptor.OverflowY == OverflowModes::Stretch && contentHeight > descriptor.Height)
    {
        currentHeight = contentHeight;
    }

    if (descriptor.OverflowX == OverflowModes::Scroll)
    {
        scrollProvider->InitializeScrollbar(hWindow, contentWidth, currentWidth, SB_HORZ);
    }

    if (descriptor.OverflowY == OverflowModes::Scroll)
    {
        scrollProvider->InitializeScrollbar(hWindow, contentHeight, currentHeight, SB_VERT);
    }

    InvalidateRect(hWindow, NULL, FALSE);
    MoveWindow(hWindow, descriptor.X, descriptor.Y, currentWidth, currentHeight, FALSE);

    if (preserveVerticalScroll)
    {
        scrollProvider->SetScrollPosition(hWindow, SB_VERT, verticalScroll);
    }
}

DWORD Window::GetScrollStyle()
{
    int scrollStyle = 0;

    if (descriptor.OverflowX == OverflowModes::Scroll)
    {
        scrollStyle |= WS_HSCROLL;
    }

    if (descriptor.OverflowY == OverflowModes::Scroll)
    {
        scrollStyle |= WS_VSCROLL;
    }

    return scrollStyle;
}

HWND Window::GetHandle()
{
    return hWindow;
}

HINSTANCE Window::GetInstance()
{
    return hInstance;
}

int Window::GetWidth()
{
    return currentWidth;
}

int Window::GetHeight()
{
    return currentHeight;
}

void Window::Show()
{
    ShowWindow(hWindow, SW_SHOW);
}

void Window::Hide()
{
    ShowWindow(hWindow, SW_HIDE);
}

LRESULT CALLBACK Window::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    Window* instance = (Window*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

    int zDelta;
    switch (message)
    {

    case WM_HSCROLL:
        instance->scrollProvider->ProcessHorizontalScroll(hWnd, wParam, lParam);
        break;

    case WM_VSCROLL:
        instance->scrollProvider->ProcessVerticalScroll(hWnd, wParam, lParam);
        break;

    case WM_MOUSEWHEEL:

        zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
        if (zDelta < 0)
            SendMessage(instance->GetHandle(), WM_VSCROLL, SB_LINEDOWN, NULL);
        else
            SendMessage(instance->GetHandle(), WM_VSCROLL, SB_LINEUP, NULL);

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);

    }

    return 0;
}

Window::~Window()
{
}