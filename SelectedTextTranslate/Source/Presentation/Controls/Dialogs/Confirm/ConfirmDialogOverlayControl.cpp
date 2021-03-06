#include "Presentation\Controls\Dialogs\Confirm\ConfirmDialogOverlayControl.h"
#include "Infrastructure\ErrorHandling\ExceptionHelper.h"

ConfirmDialogOverlayControl::ConfirmDialogOverlayControl(CommonContext* context, View* parentWindow)
    : ControlView(context, parentWindow)
{
    this->ClassName = L"STT_CONFIRM_DIALOG_OVERLAY";
    this->IsLayered = true;
    this->backgroundBrush = RenderingProvider->CreateCustomBrush(Colors::White);
}

void ConfirmDialogOverlayControl::Initialize()
{
    ControlView::Initialize();

    AssertCriticalWinApiResult(SetLayeredWindowAttributes(Handle, 0, 175, LWA_ALPHA));
}

void ConfirmDialogOverlayControl::SetSize(Size size)
{
    AssertViewNotInitialized();
    Layout = LayoutDescriptor::CreateFixedLayoutDescriptor(Point(0, 0), size);
}

Size ConfirmDialogOverlayControl::RenderContent(Renderer* renderer)
{
    renderer->DrawRect(Rect(Point(0, 0), State.GetSize()), backgroundBrush);
    return State.GetSize();
}

ConfirmDialogOverlayControl::~ConfirmDialogOverlayControl()
{
    delete backgroundBrush;
}