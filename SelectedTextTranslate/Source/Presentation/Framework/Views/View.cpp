#include "Presentation\Framework\Views\View.h"
#include "Infrastructure\ErrorHandling\Exceptions\SelectedTextTranslateException.h"
#include "Infrastructure\ErrorHandling\Exceptions\SelectedTextTranslateFatalException.h"
#include "Infrastructure\ErrorHandling\ExceptionHelper.h"

View::View(ViewContext* context)
    : NativeWindowHolder()
{
    if(context == nullptr)
    {
        throw SelectedTextTranslateFatalException(L"View context must be provided.");
    }

    Context = context;
    ScaleProvider = context->GetScaleProvider();
    ScrollProvider = context->GetScrollProvider();
    RenderingProvider = context->GetRenderingProvider();
    RenderingContext = context->GetRenderingContext();
    DeviceContextProvider = context->GetDeviceContextProvider();

    State = new ViewStateDescriptor(DeviceContextProvider);
    activeChildViews = vector<View*>();
    destroyedChildViews = vector<View*>();

    ClassName = nullptr;
    Name = wstring();
}

void View::Initialize()
{
    if(State->GetLayout().IsEmpty())
    {
        throw SelectedTextTranslateException(L"Layout must be set for view");
    }

    State->MakeVisible();

    NativeWindowHolder::Initialize();

    State->SetViewState(ViewStates::Initialized);
}

void View::SpecifyWindowClass(WNDCLASSEX* windowClass)
{
    // Reserve for scroll provider
    windowClass->cbWndExtra = 2 * sizeof(LONG_PTR);
}

Rect View::GetWindowRectangle() const
{
    Point offset = GetInitialViewOffset();
    Point position = Point(
        State->GetPosition().GetX() - offset.GetX(),
        State->GetPosition().GetY() - offset.GetY());
    return Rect(position, State->GetWindowSize());
}

Point View::GetInitialViewOffset() const
{
    return Point(0, 0);
}

DWORD View::GetWindowStyle() const
{
    int scrollStyle = 0;

    if (State->GetLayout().GetOverflowX() == OverflowModes::Scroll)
    {
        scrollStyle |= WS_HSCROLL;
    }

    if (State->GetLayout().GetOverflowY() == OverflowModes::Scroll)
    {
        scrollStyle |= WS_VSCROLL;
    }

    return NativeWindowHolder::GetWindowStyle() | scrollStyle | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
}

void View::InitializeAndRender(bool preserveScrolls)
{
    Initialize();
    Render(preserveScrolls);
}

void View::Render(bool preserveScrolls)
{
    printf("Render: %ls, %ls\n", Name.c_str(), ClassName);
    AssertViewInitialized();

    RenderingContext->BeginRender(this);

    State->SetViewState(ViewStates::Rendering);
    State->ResetToLayout();

    RenderToBuffer();

    if (RenderingContext->IsRenderingRoot(this))
    {
        ApplyViewState(preserveScrolls);
    }

    State->SetViewState(ViewStates::Rendered);
    RenderingContext->EndRender(this);
}

void View::RenderToBuffer()
{
    Renderer* renderer = RenderingContext->GetRenderer();

    RenderContent(renderer);

    Size childViewsSize;
    for (View* childView : activeChildViews)
    {
        if(childView->IsVisible())
        {
            childViewsSize = childViewsSize.Max(Size(
                childView->GetBoundingRect().GetRight(),
                childView->GetBoundingRect().GetBottom()));
        }
    }

    State->SetContentSize(childViewsSize);
    State->UpdateContent(renderer);

    RenderingContext->ReleaseRenderer(renderer);
}

void View::ApplyViewState(bool preserveScrolls)
{
    // Child windows should be destroyed first
    DestroyChildViews(destroyedChildViews);

    if (IsVisible())
    {
        Show();

        ApplyViewPosition(preserveScrolls);

        // Important to draw child windows first
        for (View* childView : activeChildViews)
        {
            childView->ApplyViewState(preserveScrolls);
        }

        Draw(false);
    }
    else
    {
        Hide();
    }
}

void View::ApplyViewPosition(bool preserveScrolls)
{
    int verticalScrollPosition = 0;
    int horizontalScrollPosition = 0;

    if (preserveScrolls)
    {
        verticalScrollPosition = ScrollProvider->GetCurrentScrollPostion(this, ScrollBars::Vertical);
        horizontalScrollPosition = ScrollProvider->GetCurrentScrollPostion(this, ScrollBars::Horizontal);
    }

    Point offset = GetInitialViewOffset();
    AssertCriticalWinApiResult(MoveWindow(
        Handle,
        State->GetPosition().GetX() - offset.GetX(),
        State->GetPosition().GetY() - offset.GetY(),
        State->GetWindowSize().GetWidth(),
        State->GetWindowSize().GetHeight(),
        FALSE));

    // Important to initialize scroll only after window has been moved
    ScrollProvider->InitializeScrollbars(
        this,
        State->GetLayout().GetOverflowX() == OverflowModes::Scroll,
        State->GetLayout().GetOverflowY() == OverflowModes::Scroll,
        verticalScrollPosition,
        horizontalScrollPosition);

    // Render scroll immediately
    SendMessage(Handle, WM_NCPAINT, NULL, NULL);
}

void View::Draw(bool drawChildren)
{
    AssertViewInitialized();

    if (!IsVisible())
    {
        return;
    }

    printf("Draw: %ls, %ls, children: %d\n", Name.c_str(), ClassName, drawChildren);

    if (drawChildren)
    {
        // Important to draw child windows before drawing parent window.
        // Otherwise, WM_CLIPCHILDREN style will cause redraw of the parent window after the child windows draw.
        DrawChildViews();
    }

    HDC deviceContext = GetDC(Handle);
    AssertCriticalWinApiResult(deviceContext);

    HDC tempDc = DeviceContextProvider->CreateDeviceContext(State->GetWindowSize());
    DeviceContextProvider->ClearDeviceContext(tempDc, State->GetWindowSize(), (HBRUSH)GetStockObject(WHITE_BRUSH));
    State->GetDeviceContextBuffer()->Render(tempDc, State->GetDeviceContextBuffer()->GetSize());
    DeviceContextProvider->CopyDeviceContext(tempDc, deviceContext, State->GetWindowSize());
    DeviceContextProvider->DeleteDeviceContext(tempDc);

    AssertCriticalWinApiResult(ReleaseDC(Handle, deviceContext));
}

void View::DrawChildViews()
{
    for (size_t i = 0; i < activeChildViews.size(); ++i)
    {
        View* childView = activeChildViews[i];
        childView->Draw(true);
    }
}

void View::AddChildView(View* childView)
{
    activeChildViews.push_back(childView);
}

void View::DestroyChildViews()
{
    destroyedChildViews.insert(destroyedChildViews.end(), activeChildViews.begin(), activeChildViews.end());
    activeChildViews = vector<View*>();
}

void View::DestroyChildViews(vector<View*>& childViews) const
{
    for (size_t i = 0; i < childViews.size(); ++i)
    {
        delete childViews[i];
    }

    childViews.clear();
    childViews.resize(0);
}

Size View::GetAvailableClientSize() const
{
    RECT clientRect;
    AssertCriticalWinApiResult(GetClientRect(Handle, &clientRect));
    Size currentClientSize = Size(clientRect.right, clientRect.bottom);

    // Do not count showed scrollbars towards client size
    Size availablClientSize = Size(
        currentClientSize.GetWidth() + ScrollProvider->GetScrollBarSize(this, ScrollBars::Vertical),
        currentClientSize.GetHeight() + ScrollProvider->GetScrollBarSize(this, ScrollBars::Horizontal));

    return availablClientSize;
}


Size View::GetContentSize() const
{
    return State->GetContentSize();
}

Rect View::GetBoundingRect() const
{
    return State->GetBoundingRect();
}

void View::MakeVisible() const
{
    State->MakeVisible();
}

void View::MakeHidden() const
{
    State->MakeHidden();
}

bool View::IsVisible() const
{
    return State->IsVisible();
}

void View::Show()
{
    AssertViewInitialized();
    MakeVisible();
    ShowWindow(GetHandle(), SW_SHOW);
}

void View::Hide()
{
    AssertViewInitialized();
    MakeHidden();
    ShowWindow(GetHandle(), SW_HIDE);
}

LRESULT View::ExecuteWindowProcedure(UINT message, WPARAM wParam, LPARAM lParam)
{
    try
    {
        ExceptionHelper::SetupStructuredExceptionsTranslation();
        return WindowProcedure(message, wParam, lParam);
    }
    catch (const SelectedTextTranslateException& error)
    {
        ExceptionHelper::HandleNonFatalException(Context->GetLogger(), Context->GetErrorHandler(), L"Error occurred.", error);
    }
    catch (...)
    {
        Context->GetErrorHandler()->HandleFatalException();
    }

    return -1;
}

LRESULT View::WindowProcedure(UINT message, WPARAM wParam, LPARAM lParam)
{
    ScrollProvider->ProcessScrollMessages(this, message, wParam, lParam);

    switch (message)
    {
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            AssertCriticalWinApiResult(BeginPaint(GetHandle(), &ps));
            Draw(false);
            EndPaint(GetHandle(), &ps);
            break;
        }

        // Prevent of the background erase reduces flickering before Draw.
        case WM_ERASEBKGND:
            break;

        default:
            return CallBaseWindowProcedure(message, wParam, lParam);
    }

    return 0;
}

void View::AssertViewInitialized() const
{
    if (State->GetViewState() == ViewStates::New)
    {
        throw SelectedTextTranslateFatalException(L"View has not been initialized.");
    }
}

void View::AssertViewNotInitialized() const
{
    if (State->GetViewState() != ViewStates::New)
    {
        throw SelectedTextTranslateFatalException(L"View has been already initialized.");
    }
}

View::~View()
{
    delete State;

    DestroyChildViews(activeChildViews);
    DestroyChildViews(destroyedChildViews);
}