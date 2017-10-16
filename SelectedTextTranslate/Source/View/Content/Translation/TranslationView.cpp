#include "View\Content\Translation\TranslationView.h"

TranslationView::TranslationView(ViewContext* context, View* parentView, ModelHolder<TranslateResult>* modelHolder)
    : ComponentView(context, parentView, modelHolder)
{
    this->headerHeight = context->GetScaleProvider()->Scale(50);
    this->separatorHeight = context->GetScaleProvider()->Scale(1);

    this->headerComponent = nullptr;
    this->translateResultComponent = nullptr;
}

void TranslationView::Initialize()
{
    ComponentView::Initialize();

    WindowDescriptor headerWindowDescriptor = WindowDescriptor::CreateWindowDescriptor(
        Point(0, 0),
        Size(0, headerHeight),
        OverflowModes::Stretch,
        OverflowModes::Fixed);
    headerComponent = new HeaderComponent(context, this, modelHolder);
    headerComponent->SetDescriptor(headerWindowDescriptor);
    headerComponent->OnTranslateSuggestion.Subscribe(&OnTranslateSuggestion);
    headerComponent->OnForceTranslation.Subscribe(&OnForceTranslation);
    headerComponent->Initialize();

    WindowDescriptor translateResultWindowDescriptor = WindowDescriptor::CreateWindowDescriptor(
        Point(0, headerComponent->GetBoundingRect().GetBottom() + separatorHeight),
        Size(0, 0),
        OverflowModes::Stretch,
        OverflowModes::Stretch);
    translateResultComponent = new TranslateResultComponent(context, this, modelHolder);
    translateResultComponent->SetDescriptor(translateResultWindowDescriptor);
    translateResultComponent->Initialize();
}

Size TranslationView::RenderContent(Renderer* renderer, TranslateResult model)
{
    headerComponent->Render();

    Size contentSize;

    if (model.IsEmptyResult())
    {
        translateResultComponent->MakeHidden();

        Size backgroundSize = Size(
            max(parentView->GetSize().GetWidth(), headerComponent->GetBoundingRect().GetWidth()),
            parentView->GetSize().GetHeight() - headerHeight);
        renderer->DrawRect(Rect(Point(0, headerHeight), backgroundSize), backgroundBrush);

        contentSize = headerComponent->GetBoundingRect().GetSize();
    }
    else
    {
        translateResultComponent->MakeVisible();
        translateResultComponent->Render();

        contentSize = Size(
            max(headerComponent->GetBoundingRect().GetWidth(), translateResultComponent->GetBoundingRect().GetWidth()),
            headerComponent->GetBoundingRect().GetHeight() + translateResultComponent->GetBoundingRect().GetHeight());
    }

    RenderSeparator(renderer, max(GetSize().GetWidth(), contentSize.GetWidth()));

    return contentSize;
}

void TranslationView::Resize()
{
    Size parentSize = parentView->GetSize();
    descriptor.SetWindowSize(parentSize);

    nativeStateDescriptor.EnsureSize(parentSize);

    Size bufferingDeviceContextSize = deviceContextBuffer->GetSize();
    bufferingDeviceContextSize = Size(
        max(parentSize.GetWidth(), bufferingDeviceContextSize.GetWidth()),
        max(parentSize.GetHeight(), bufferingDeviceContextSize.GetHeight()));
    deviceContextBuffer->Resize(bufferingDeviceContextSize);

    ApplyNativeState(true);

    Renderer* renderer = context->GetRenderingContext()->GetRenderer();

    if(modelHolder->GetModel().IsEmptyResult())
    {
        renderer->DrawRect(Rect(0, headerHeight, nativeStateDescriptor.GetSize().GetWidth(), nativeStateDescriptor.GetSize().GetHeight() - headerHeight), lightGrayBrush);
    }

    RenderSeparator(renderer, max(contentSize.GetWidth(), nativeStateDescriptor.GetSize().GetWidth()));

    renderer->Render(deviceContextBuffer);
    context->GetRenderingContext()->ReleaseRenderer(renderer);

    Draw();
}

void TranslationView::RenderSeparator(Renderer* renderer, int width) const
{
    renderer->DrawRect(Rect(0, headerHeight, width, separatorHeight), lightGrayBrush);
}