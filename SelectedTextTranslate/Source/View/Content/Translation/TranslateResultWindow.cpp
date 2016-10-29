﻿#include "View\Content\Translation\TranslateResultWindow.h"
#include "View\Controls\Buttons\HoverTextButtonWindow.h"
#include "Infrastructure\ErrorHandling\ExceptionHelper.h"

TranslateResultWindow::TranslateResultWindow(WindowContext* context, WindowDescriptor descriptor, Window* parentWindow)
: ContentWindow(context, descriptor, parentWindow)
{
    this->fontUnderscored = nullptr;
    this->OnExpandTranslationResult = Subscribeable<int>();
}

void TranslateResultWindow::Initialize()
{
    ContentWindow::Initialize();

    fontUnderscored = context->GetRenderingContext()->CreateCustomFont(windowHandle, FontSizes::Small, false, true);
}

void TranslateResultWindow::SetModel(TranslateResult translateResult)
{
    AssertWindowInitialized();

    this->translateResult = translateResult;
}

Size TranslateResultWindow::RenderContent(Renderer* renderer)
{
    ContentWindow::RenderContent(renderer);
    DestroyChildWindows();

    int curY = lineHeight;

    for (size_t i = 0; i < translateResult.GetTranslateCategories().size(); ++i)
    {
        TranslateResultCategory category = translateResult.GetTranslateCategories()[i];

        // Draw category header
        Point baseFormBottomRight = renderer->PrintText(category.GetBaseForm(), fontNormal, Colors::Black, Point(paddingX, curY));

        wstring partOfSpeech = L" - " + wstring(category.GetPartOfSpeech());
        renderer->PrintText(partOfSpeech.c_str(), fontItalic, Colors::Gray, Point(baseFormBottomRight.X + 1, curY));

        vector<TranslateResultCategoryEntry> showedEntries(0);
        if (category.IsExtendedList())
        {
            showedEntries = category.GetEntries();
        }
        else
        {
            for (size_t j = 0; j < category.GetEntries().size(); ++j)
            {
                if (category.GetEntries()[j].GetScore() < 0.003 && j >= 7)
                {
                    continue;
                }

                showedEntries.push_back(category.GetEntries()[j]);
            }
        }

        // Draw words
        for (size_t j = 0; j < showedEntries.size(); ++j)
        {
            curY += lineHeight;

            TranslateResultCategoryEntry entry = category.GetEntries()[j];
            Point wordBottomRight = renderer->PrintText(entry.GetWord(), fontNormal, Colors::Black, Point(paddingX * 3, curY));

            // Draw reverse translation
            if (entry.GetReverseTranslations().size() != 0)
            {
                wordBottomRight = renderer->PrintText(L" - ", fontNormal, Colors::Gray, Point(wordBottomRight.X + 2, curY));
                for (size_t k = 0; k < entry.GetReverseTranslations().size(); ++k)
                {
                    wstring text = wstring(entry.GetReverseTranslations()[k]);
                    if (k != entry.GetReverseTranslations().size() - 1)
                    {
                        text += L", ";
                    }
                    wordBottomRight = renderer->PrintText(text.c_str(), fontItalic, Colors::Gray, Point(wordBottomRight.X, curY));
                }
            }

            int k = entry.GetScore() >= 0.05 ? 0 : (entry.GetScore() >= 0.0025 ? 1 : 2);
            int rateUnit = 8;
            int strokeHeight = renderer->GetFontStrokeHeight(fontNormal);

            Rect rect;
            rect.Y = curY - strokeHeight + 2;
            rect.X = paddingX + k * rateUnit;
            rect.Width = (3 - k) * rateUnit;
            rect.Height = strokeHeight - 2;

            renderer->DrawRect(rect, grayBrush);
        }

        curY = CreateExpandButton(category, i, showedEntries.size(), curY , renderer);

        curY += lineHeight + lineHeight / 2;
    }

    renderer->IncreaseWidth(paddingX);
    renderer->IncreaseHeight(lineHeight);

    return renderer->GetScaledSize();
}

int TranslateResultWindow::CreateExpandButton(
    TranslateResultCategory category,
    int categoryIndex,
    int showedCount,
    int curY,
    Renderer* renderer)
{
    DWORD hiddenCount = category.GetEntries().size() - showedCount;

    if (category.IsExtendedList() || hiddenCount > 0) {
        wstring text;

        if (!category.IsExtendedList()) {
            if (hiddenCount == 1) {
                text = L"show " + to_wstring(hiddenCount) + L" more result";
            }
            else {
                text = L"show " + to_wstring(hiddenCount) + L" more results";
            }
        }
        else {
            text = L"show less results";
        }

        int underscoredFontAscent = renderer->GetFontAscent(fontUnderscored);
        HoverTextButtonWindow* expandButton = new HoverTextButtonWindow(
            context,
            WindowDescriptor::CreateStretchWindowDescriptor(Point(paddingX * 3, curY + 7)),
            this,
            fontUnderscored,
            Colors::Gray,
            Colors::Black,
            text);

        expandButton->OnClick.Subscribe([categoryIndex, this]() -> void
        {
            return OnExpandTranslationResult.Notify(categoryIndex);
        });

        AddChildWindow(expandButton);

        renderer->IncreaseHeight(expandButton->GetSize().Height);
        curY += underscoredFontAscent;
    }

    return curY;
}

TranslateResultWindow::~TranslateResultWindow()
{
    AssertCriticalWinApiResult(DeleteObject(this->fontUnderscored));
}