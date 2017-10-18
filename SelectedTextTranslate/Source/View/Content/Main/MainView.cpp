#include "View\Content\Main\MainView.h"
#include "Infrastructure\ErrorHandling\ExceptionHelper.h"
#include "Infrastructure\ErrorHandling\Exceptions\SelectedTextTranslateFatalException.h"
#include "Utilities\StringUtilities.h"
#include "View\Controls\Dialogs\Confirm\ConfirmDialogWindow.h"
#include "View\Content\Main\Enums\ApplicationViews.h"
#include "View\Content\Translation\TranslationComponent.h"
#include "View\Framework\Providers\ScrollProvider.h"

MainView::MainView(ViewContext* context)
    : View(context)
{
    this->className = L"STT_MAIN";
    this->applicationView = ApplicationViews::None;

    this->dictionaryComponent = nullptr;
    this->translationWindow = nullptr;
    this->settingsWindow = nullptr;
    this->confirmDialogWindow = nullptr;

    this->applicationViewDescriptors = map<ApplicationViews, ViewDescriptor>();
}

void MainView::SetDescriptor(WindowDescriptor descriptor)
{
    View::SetDescriptor(descriptor);

    applicationViewDescriptors[ApplicationViews::Settings] = ViewDescriptor(descriptor, false);
    applicationViewDescriptors[ApplicationViews::Dictionary] = ViewDescriptor(descriptor, true);
    applicationViewDescriptors[ApplicationViews::TranslateResult] = ViewDescriptor(descriptor, true);
    minSize = descriptor.GetSize();
}

void MainView::Initialize()
{
    View::Initialize();

    windowHandle = CreateWindowEx(
        WS_EX_TOOLWINDOW,
        className,
        nullptr,
        WS_SIZEBOX | WS_POPUP | WS_CLIPCHILDREN | GetScrollStyle(),
        descriptor.GetPosition().GetX(),
        descriptor.GetPosition().GetY(),
        descriptor.GetSize().GetWidth(),
        descriptor.GetSize().GetHeight(),
        nullptr,
        nullptr,
        context->GetInstance(),
        nullptr);
    AssertCriticalWinApiResult(windowHandle);
    SetWindowLongPtr(windowHandle, GWLP_USERDATA, (LONG_PTR)this);

    CreateChildViews();

    Minimize();

    context->GetErrorHandler()->OnErrorShow.Subscribe(bind(&MainView::Minimize, this));
    context->Get<MessageBus>()->OnConfirmRequested.Subscribe(bind(&MainView::ShowConfirmDialog, this, placeholders::_1, placeholders::_2));
}

void MainView::CreateChildViews()
{
    DestroyChildViews();

    translationWindow = new TranslationComponent(context, this);
    SetViewWindowDescriptor(translationWindow->GetView(), ApplicationViews::TranslateResult);
    translationWindow->MakeHidden();
    translationWindow->Initialize();

    dictionaryComponent = new DictionaryComponent(context, this);
    SetViewWindowDescriptor(dictionaryComponent->GetView(), ApplicationViews::Dictionary);
    dictionaryComponent->OnShowTranslation.Subscribe([this](wstring input)
    {
        this->translationWindow->Translate(input, false);
        SetApplicationView(ApplicationViews::TranslateResult);
    });
    dictionaryComponent->GetView()->MakeHidden();
    dictionaryComponent->GetView()->Initialize();

    settingsWindow = new SettingsComponent(context, this);
    SetViewWindowDescriptor(settingsWindow->GetView(), ApplicationViews::Settings);
    settingsWindow->MakeHidden();
    settingsWindow->Initialize();

    confirmDialogWindow = new ConfirmDialogWindow(context, this);
    confirmDialogWindow->SetDescriptor(WindowDescriptor::CreateFixedWindowDescriptor(Point(0, 0), GetClientSize()));
    confirmDialogWindow->MakeHidden();
    confirmDialogWindow->Initialize();
}

void MainView::SetViewWindowDescriptor(View* viewWindow, ApplicationViews view)
{
    WindowDescriptor windowDescriptor = WindowDescriptor::CreateWindowDescriptor(
        Point(0, 0),
        applicationViewDescriptors[view].GetWindowDescriptor().GetSize(),
        OverflowModes::Stretch,
        OverflowModes::Stretch);
    viewWindow->SetDescriptor(windowDescriptor);
}

void MainView::SpecifyWindowClass(WNDCLASSEX* windowClass)
{
    windowClass->hIcon = LoadIcon(context->GetInstance(), MAKEINTRESOURCE(IDI_APP_ICON));
    AssertCriticalWinApiResult(windowClass->hIcon);

    windowClass->hIconSm = LoadIcon(context->GetInstance(), MAKEINTRESOURCE(IDI_APP_ICON));
    AssertCriticalWinApiResult(windowClass->hIconSm);

    windowClass->hCursor = LoadCursor(nullptr, IDC_ARROW);
    AssertCriticalWinApiResult(windowClass->hCursor);

    windowClass->hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    AssertCriticalWinApiResult(windowClass->hbrBackground);
}

void MainView::Minimize()
{
    Hide();
}

void MainView::Maximize()
{
    Show();
    SwitchToThisWindow(windowHandle, TRUE);
}

void MainView::SetApplicationView(ApplicationViews applicationView)
{
    AssertViewInitialized();

    this->applicationView = applicationView;

    descriptor = applicationViewDescriptors[applicationView].GetWindowDescriptor();
    nativeStateDescriptor.SetPosition(descriptor.GetPosition());
    nativeStateDescriptor.SetSize(descriptor.GetSize());

    Maximize();
    Render();
}

void MainView::Translate(wstring input)
{
    this->translationWindow->Translate(input, true);
    SetApplicationView(ApplicationViews::TranslateResult);
}

Size MainView::RenderContent(Renderer* renderer)
{
    if (applicationView == ApplicationViews::None)
    {
        throw SelectedTextTranslateFatalException(L"View must set before rendering.");
    }

    for (size_t i = 0; i < activeChildViews.size(); ++i)
    {
        activeChildViews[i]->MakeHidden();
    }

    View* windowToShow = GetViewToShow();

    windowToShow->MakeVisible();
    windowToShow->Render();

    return windowToShow->GetContentSize();
}

void MainView::Scale(double scaleFactorAdjustment)
{
    if (!scaleProvider->IsScalingAllowed(scaleFactorAdjustment))
    {
        return;
    }

    // TODO: iterate thru map
    ScaleViewDescriptor(ApplicationViews::Settings, scaleFactorAdjustment);
    ScaleViewDescriptor(ApplicationViews::Dictionary, scaleFactorAdjustment);
    ScaleViewDescriptor(ApplicationViews::TranslateResult, scaleFactorAdjustment);
    minSize = Size(
        scaleProvider->Rescale(minSize.GetWidth(), scaleFactorAdjustment),
        scaleProvider->Rescale(minSize.GetHeight(), scaleFactorAdjustment));

    descriptor = applicationViewDescriptors[applicationView].GetWindowDescriptor();
    nativeStateDescriptor.SetPosition(descriptor.GetPosition());
    nativeStateDescriptor.SetSize(descriptor.GetSize());

    scaleProvider->AdjustScaleFactor(scaleFactorAdjustment);

    deviceContextBuffer->Resize(nativeStateDescriptor.GetSize());

    CreateChildViews();
    Render();
}

void MainView::ScaleViewDescriptor(ApplicationViews applicationView, double scaleFactorAdjustment)
{
    WindowDescriptor windowDescriptor = applicationViewDescriptors[applicationView].GetWindowDescriptor();

    int scaledWidth = scaleProvider->Rescale(windowDescriptor.GetSize().GetWidth(), scaleFactorAdjustment);
    int scaledHeight = scaleProvider->Rescale(windowDescriptor.GetSize().GetHeight(), scaleFactorAdjustment);

    windowDescriptor.SetPosition(Point(
        windowDescriptor.GetPosition().GetX() - scaledWidth + windowDescriptor.GetSize().GetWidth(),
        windowDescriptor.GetPosition().GetY() - scaledHeight + windowDescriptor.GetSize().GetHeight()));

    windowDescriptor.SetWindowSize(Size(scaledWidth, scaledHeight));

    applicationViewDescriptors[applicationView].SetWindowDescriptor(windowDescriptor);
}

void MainView::Resize()
{
    if (viewState == ViewStates::Rendering)
    {
        return;
    }

    RECT windowRect;
    AssertCriticalWinApiResult(GetWindowRect(windowHandle, &windowRect));
    int newWidth = windowRect.right - windowRect.left;
    int newHeight = windowRect.bottom - windowRect.top;

    if (descriptor.GetSize().GetWidth() == newWidth && descriptor.GetSize().GetHeight() == newHeight)
    {
        return;
    }

    descriptor.SetWindowSize(Size(newWidth, newHeight));
    nativeStateDescriptor.SetSize(descriptor.GetSize());

    descriptor.SetPosition(Point(windowRect.left, windowRect.top));
    nativeStateDescriptor.SetPosition(descriptor.GetPosition());

    deviceContextBuffer->Resize(nativeStateDescriptor.GetSize());

    // Clear background
    Renderer* renderer = renderingContext->GetRenderer();
    renderer->Render(deviceContextBuffer);
    renderingContext->ReleaseRenderer(renderer);

    View* currentWindow = GetViewToShow();
    currentWindow->Resize();
    contentSize = currentWindow->GetContentSize();

    applicationViewDescriptors[applicationView].SetWindowDescriptor(descriptor);

    ApplyNativeState(true);
}

View* MainView::GetViewToShow() const
{
    if (applicationView == ApplicationViews::TranslateResult)
    {
        return translationWindow->GetView();
    }

    if (applicationView == ApplicationViews::Dictionary)
    {
        return dictionaryComponent->GetView();
    }

    if (applicationView == ApplicationViews::Settings)
    {
        return settingsWindow->GetView();
    }

    throw SelectedTextTranslateFatalException(StringUtilities::Format(L"Unsupported view: %d", applicationView));
}

void MainView::ShowConfirmDialog(wstring title, function<void()> onConfirm)
{
    confirmDialogWindow->SetTitle(title);
    confirmDialogWindow->OnConfirm.UnsubscribeAll();
    confirmDialogWindow->OnConfirm.Subscribe(onConfirm);
    confirmDialogWindow->OnConfirm.Subscribe(bind(&MainView::ApplyViewPosition, this, true));
    confirmDialogWindow->OnCancel.UnsubscribeAll();
    confirmDialogWindow->OnCancel.Subscribe(bind(&MainView::ApplyViewPosition, this, true));
    confirmDialogWindow->Render();
    confirmDialogWindow->Show();
    scrollProvider->HideScrollbars(this);
}

bool MainView::IsResizeLocked()
{
    return confirmDialogWindow->IsVisible() || !applicationViewDescriptors[applicationView].IsResizeable();
}

LRESULT MainView::WindowProcedure(UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_SIZE:
    {
        Resize();
        return TRUE;
    }

    case WM_SETCURSOR:
    {
        if (IsResizeLocked())
        {
            SetCursor(LoadCursor(nullptr, IDC_ARROW));
            return TRUE;
        }

        return View::WindowProcedure(message, wParam, lParam);
    }

    case WM_SIZING:
    {
        RECT &newSize = *(LPRECT)lParam;
        int newWidth = newSize.right - newSize.left;
        int newHeight = newSize.bottom - newSize.top;

        if (IsResizeLocked() || newWidth < minSize.GetWidth() || newHeight < minSize.GetHeight())
        {
            RECT windowRect;
            GetWindowRect(windowHandle, &windowRect);
            newSize = windowRect;
            return 0;
        }

        return TRUE;
    }

    case WM_SYSCOMMAND:
    {
        switch (wParam & 0xfff0)
        {
        case SC_MINIMIZE:
        case SC_CLOSE:
            Minimize();
            return 0;
        default:
            break;
        }

        return View::WindowProcedure(message, wParam, lParam);
    }

    case WM_HOTKEY:
    {
        OnHotkey.Notify(wParam);
        return View::WindowProcedure(message, wParam, lParam);
    }

    case WM_ACTIVATE:
    {
        if (wParam == WA_INACTIVE)
        {
            Minimize();
        }
        return TRUE;
    }

    case WM_SHOWWINDOW:
    {
        OnVisibilityChanged.Notify(wParam == TRUE);
        break;
    }

    default:
        return View::WindowProcedure(message, wParam, lParam);
    }

    return 0;
}