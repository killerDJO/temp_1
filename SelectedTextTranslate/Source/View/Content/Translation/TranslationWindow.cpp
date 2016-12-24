#include "View\Content\Translation\TranslationWindow.h"
#include "Infrastructure\ErrorHandling\ExceptionHelper.h"

TranslationWindow::TranslationWindow(WindowContext* context, Window* parentWindow)
    : ContentWindow(context, parentWindow)
{
    this->OnPlayText = Subscribeable<>();
    this->OnForceTranslation = Subscribeable<>();
    this->OnTranslateSuggestion = Subscribeable<>();
    this->OnExpandTranslationResult = Subscribeable<int>();

    this->headerHeight = context->GetScaleProvider()->Scale(50);
    this->separatorHeight = context->GetScaleProvider()->Scale(1);

    this->headerWindow = nullptr;
    this->translateResultWindow = nullptr;
}

void TranslationWindow::Initialize()
{
    ContentWindow::Initialize();

    WindowDescriptor headerWindowDescriptor = WindowDescriptor::CreateWindowDescriptor(
        Point(0, 0),
        Size(0, headerHeight),
        OverflowModes::Stretch,
        OverflowModes::Fixed);
    headerWindow = new HeaderWindow(context, this);
    headerWindow->SetDescriptor(headerWindowDescriptor);
    headerWindow->OnPlayText.Subscribe(&OnPlayText);
    headerWindow->OnTranslateSuggestion.Subscribe(&OnTranslateSuggestion);
    headerWindow->OnForceTranslation.Subscribe(&OnForceTranslation);
    AddChildWindow(headerWindow);

    WindowDescriptor translateResultWindowDescriptor = WindowDescriptor::CreateWindowDescriptor(
        Point(0, headerWindow->GetBoundingRect().GetBottom() + separatorHeight),
        Size(0, 0),
        OverflowModes::Stretch,
        OverflowModes::Stretch);
    translateResultWindow = new TranslateResultWindow(context, this);
    translateResultWindow->SetDescriptor(translateResultWindowDescriptor);
    translateResultWindow->OnExpandTranslationResult.Subscribe(&OnExpandTranslationResult);
    AddChildWindow(translateResultWindow);
}

void TranslationWindow::SetModel(TranslateResult translateResult)
{
    ModelHolder<TranslateResult>::SetModel(translateResult);
    this->headerWindow->SetModel(translateResult);
    this->translateResultWindow->SetModel(translateResult);
}

Size TranslationWindow::RenderContent(Renderer* renderer)
{
    headerWindow->Render();

    Size contentSize;

    if (model.IsEmptyResult())
    {
        translateResultWindow->MakeHidden();

        Size backgroundSize = Size(
            max(parentWindow->GetSize().GetWidth(), headerWindow->GetSize().GetWidth()),
            parentWindow->GetSize().GetHeight() - headerHeight);
        renderer->DrawRect(Rect(Point(0, headerHeight), backgroundSize), backgroundBrush);

        contentSize = headerWindow->GetSize();
    }
    else
    {
        translateResultWindow->MakeVisible();
        translateResultWindow->Render();

        contentSize = Size(
            max(headerWindow->GetSize().GetWidth(), translateResultWindow->GetSize().GetWidth()),
            headerWindow->GetSize().GetHeight() + translateResultWindow->GetSize().GetHeight());
    }

    RenderSeparator(renderer, max(currentWindowSize.GetWidth(), contentSize.GetWidth()));

    return contentSize;
}

void TranslationWindow::Resize()
{
    Size parentSize = parentWindow->GetSize();
    descriptor.SetWindowSize(parentSize);

    currentWindowSize = Size(
        max(parentSize.GetWidth(), currentWindowSize.GetWidth()),
        max(parentSize.GetHeight(), currentWindowSize.GetHeight()));

    Size bufferingDeviceContextSize = deviceContextBuffer->GetSize();
    bufferingDeviceContextSize = Size(
        max(parentSize.GetWidth(), bufferingDeviceContextSize.GetWidth()),
        max(parentSize.GetHeight(), bufferingDeviceContextSize.GetHeight()));
    deviceContextBuffer->Resize(bufferingDeviceContextSize);

    AssertCriticalWinApiResult(MoveWindow(windowHandle, position.GetX(), position.GetY(), currentWindowSize.GetWidth(), currentWindowSize.GetHeight(), FALSE));

    Renderer* renderer = context->GetRenderingContext()->GetRenderer();

    if(model.IsEmptyResult())
    {
        renderer->DrawRect(Rect(0, headerHeight, currentWindowSize.GetWidth(), currentWindowSize.GetHeight() - headerHeight), lightGrayBrush);
    }

    RenderSeparator(renderer, max(contentSize.GetWidth(), currentWindowSize.GetWidth()));

    renderer->Render(deviceContextBuffer);
    context->GetRenderingContext()->ReleaseRenderer(renderer);

    Draw();
}

void TranslationWindow::RenderSeparator(Renderer* renderer, int width) const
{
    renderer->DrawRect(Rect(0, headerHeight, width, separatorHeight), lightGrayBrush);
}

TranslationWindow::~TranslationWindow()
{
}