#include "stdafx.h"
#include "TranslateResultWindow.h"
#include "TranslateResult.h"

TranslateResultWindow::TranslateResultWindow(HWND parentWindow, HINSTANCE hInstance, DWORD x, DWORD y) : ContentWindow(parentWindow, hInstance, x, y, 1000, 2000){ }

POINT TranslateResultWindow::RenderDC()
{	
	ContentWindow::RenderDC();

	POINT bottomRight = { 0, 0 };

	const int categoryMargin = 10;
	int curY = 0;

	for (size_t i = 0; i < translateResult.TranslateCategories.size(); ++i)
	{
		TranslateResultDictionary category = translateResult.TranslateCategories[i];

		// Draw category header
		POINT baseFormBottomRight = Utilities::PrintText(inMemoryHDC, category.BaseForm, fontNormal, COLOR_BLACK, PADDING, curY, &bottomRight);

		if (_tcslen(category.PartOfSpeech) != 0)
		{
			Utilities::PrintText(inMemoryHDC, L" - ", fontItalic, COLOR_GRAY, baseFormBottomRight.x + 2, curY, &bottomRight);
			Utilities::PrintText(inMemoryHDC, category.PartOfSpeech, fontItalic, COLOR_GRAY, baseFormBottomRight.x + 17, curY, &bottomRight);
		}

		// Draw words
		curY += LINE_HEIGHT;
		for (size_t j = 0; j < category.Entries.size(); ++j)
		{
			TranslateResultDictionaryEntry entry = category.Entries[j];
			Utilities::PrintText(inMemoryHDC, entry.Word, fontNormal, COLOR_BLACK, PADDING * 3, curY, &bottomRight);

			int k = entry.Score >= 0.05 ? 0 : (entry.Score >= 0.003 ? 1 : 2);
			int rateUnit = 8;

			RECT rect;
			rect.right = PADDING + rateUnit * 3;
			rect.top = curY + LINE_HEIGHT / 3;
			rect.bottom = rect.top + LINE_HEIGHT / 3;
			rect.left = PADDING + k * rateUnit;
			Utilities::DrawRect(inMemoryHDC, rect, this->grayBrush, &bottomRight);
			curY += LINE_HEIGHT;
		}

		curY += categoryMargin;
	}
	return bottomRight;
}

TranslateResultWindow::~TranslateResultWindow()
{	
}                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           