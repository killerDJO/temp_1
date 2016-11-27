#include "View\Framework\Windows\ChildWindow.h"
#include "Infrastructure\ErrorHandling\ExceptionHelper.h"

ChildWindow::ChildWindow(WindowContext* context, Window* parentWindow)
    : Window(context)
{
    this->parentWindow = parentWindow;
    this->isLayered = false;
}

void ChildWindow::Initialize()
{
    Window::Initialize();

    Point offset = GetInitialWindowOffset();

    windowHandle = CreateWindow(
        className,
        nullptr,
        WS_CHILD | WS_CLIPCHILDREN,
        descriptor.GetPosition().X - offset.X,
        descriptor.GetPosition().Y - offset.Y,
        descriptor.GetWindowSize().Width,
        descriptor.GetWindowSize().Height,
        parentWindow->GetHandle(),
        nullptr,
        context->GetInstance(),
        nullptr);

    AssertCriticalWinApiResult(windowHandle);

    SetWindowLongPtr(windowHandle, GWLP_USERDATA, (LONG_PTR)this);

    if(isLayered)
    {
        SetWindowLongPtr(windowHandle, GWL_EXSTYLE, GetWindowLongPtr(windowHandle, GWL_EXSTYLE) | WS_EX_LAYERED);
        AssertCriticalWinApiResult(SetLayeredWindowAttributes(windowHandle, 0, 255, LWA_ALPHA));
    }
}

void ChildWindow::EnableLayeredMode()
{
    AssertWindowNotInitialized();
    this->isLayered = true;
}

Point ChildWindow::GetInitialWindowOffset()
{
    ScrollProvider* scrollProvider = context->GetScrollProvider();
    int offsetY = scrollProvider->GetCurrentScrollOffset(parentWindow, ScrollBars::Vertical);
    int offsetX = scrollProvider->GetCurrentScrollOffset(parentWindow, ScrollBars::Horizontal);

    return Point(offsetX, offsetY);
}

LRESULT ChildWindow::WindowProcedure(UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {

    // This message should be processed in order to enable scrolling to work.
    // Its sent to child windows after ScrollWindow on parent is called.
    case WM_MOVE:
    {
        RECT rcWindow;
        POINTS pos = MAKEPOINTS(lParam);
        AssertCriticalWinApiResult(GetWindowRect(windowHandle, &rcWindow));
        AssertCriticalWinApiResult(MoveWindow(windowHandle, pos.x, pos.y, windowSize.Width, windowSize.Height, FALSE));
        position.X = pos.x;
        position.Y = pos.y;

        return TRUE;
    }

    case WM_LBUTTONDOWN:
    {
        AssertCriticalWinApiResult(SetFocus(windowHandle));
        return TRUE;
    }

    default:
        return Window::WindowProcedure(message, wParam, lParam);
    }
}

ChildWindow::~ChildWindow()
{
}