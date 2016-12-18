#include "View\Content\Translation\HeaderWindow.h"
#include "View\Controls\Buttons\HoverIconButtonWindow.h"
#include "View\Controls\Buttons\HoverTextButtonWindow.h"
#include "Utilities\StringUtilities.h"

HeaderWindow::HeaderWindow(WindowContext* context, Window* parentWindow)
    : ContentWindow(context, parentWindow)
{
    this->OnPlayText = Subscribeable<>();
    this->OnForceTranslation = Subscribeable<>();
    this->OnTranslateSuggestion = Subscribeable<>();
}

Size HeaderWindow::RenderContent(Renderer* renderer)
{
    ContentWindow::RenderContent(renderer);
    DestroyChildWindows();

    if(!model.IsEmptyResult())
    {
        return RenderTranslationResult(renderer);
    }
    else
    {
        return RenderEmptyResult(renderer);
    }
}

Size HeaderWindow::RenderTranslationResult(Renderer* renderer)
{
    double smallFontAscent = renderer->GetFontAscent(fontSmall);

    RenderPosition renderPosition = RenderPosition(paddingX, lineHeight);

    TranslateResultSentence sentence = model.GetSentence();
    renderer->PrintText(sentence.GetTranslation(), fontHeader, Colors::Black, renderPosition);

    renderPosition = renderPosition.MoveY(lineHeight);

    double imageSize = smallFontAscent;
    HoverIconButtonWindow* audioButton = new HoverIconButtonWindow(context, this);
    audioButton->SetDimensions(renderPosition.MoveY(- imageSize + 2).GetPosition(), SizeReal(imageSize, imageSize));
    audioButton->SetHoverIconResource(IDR_AUDIO);
    audioButton->SetNormalIconResource(IDR_AUDIO_INACTIVE);
    audioButton->OnClick.Subscribe(&OnPlayText);
    AddChildWindow(audioButton);

    renderPosition = renderPosition.MoveX(imageSize + 2);
    renderPosition = renderer->PrintText(sentence.GetOrigin(), fontSmall, Colors::Gray, renderPosition);

    RenderDescriptor actionRenderDescriptor = RenderDescriptor(renderer, renderPosition.MoveX(1));
    if (model.IsInputCorrected())
    {
        PrintInputCorrectionWarning(actionRenderDescriptor, sentence.GetInput());
    }
    else if (!sentence.GetSuggestion().empty())
    {
        PrintSuggestion(actionRenderDescriptor, sentence.GetSuggestion());
    }

    renderer->IncreaseWidth(paddingX);

    return renderer->GetScaledSize();
}

Size HeaderWindow::RenderEmptyResult(Renderer* renderer) const
{
    double fontHeight = renderer->GetFontStrokeHeight(fontHeader);
    double curY = context->GetScaleProvider()->Downscale(windowSize.GetHeight() / 2) + fontHeight / 2;

    renderer->PrintText(wstring(L"No text data selected"), fontHeader, Colors::Gray, RenderPosition(paddingX, curY));

    return renderer->GetScaledSize();
}

void HeaderWindow::PrintInputCorrectionWarning(RenderDescriptor renderDescriptor, wstring originalInput)
{
    PrintHeaderAction(renderDescriptor, L"corrected from", originalInput, &OnForceTranslation);
}

void HeaderWindow::PrintSuggestion(RenderDescriptor renderDescriptor, wstring suggestion)
{
    PrintHeaderAction(renderDescriptor, L"maybe you meant", suggestion, &OnTranslateSuggestion);
}

void HeaderWindow::PrintHeaderAction(RenderDescriptor renderDescriptor, wstring actionDescription, wstring actionText, Subscribeable<>* actionCallback)
{
    TextRenderResult orignLineRenderResult = renderDescriptor.GetRenderer()->PrintText(
        StringUtilities::Format(L" (%ls ", actionDescription.c_str()),
        fontSmall,
        Colors::Gray,
        renderDescriptor.GetRenderPosition());

    double smallFontAscent = renderDescriptor.GetRenderer()->GetFontAscent(fontSmall);
    HoverTextButtonWindow* headerActionButton = new HoverTextButtonWindow(context, this);
    headerActionButton->SetFont(fontSmallUnderscored);
    headerActionButton->SetPosition(orignLineRenderResult.GetRenderPosition().MoveY(-smallFontAscent).GetPosition());
    headerActionButton->SetText(actionText);
    headerActionButton->OnClick.Subscribe(actionCallback);

    AddChildWindow(headerActionButton);

    renderDescriptor.GetRenderer()->PrintText(
        L")",
        fontSmall,
        Colors::Gray,
        PointReal(headerActionButton->GetDownscaledBoundingRect().GetRight(), orignLineRenderResult.GetBaselineY()));
}

HeaderWindow::~HeaderWindow()
{
}
