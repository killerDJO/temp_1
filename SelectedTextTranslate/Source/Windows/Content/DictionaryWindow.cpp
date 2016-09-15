#include "Windows\Content\DictionaryWindow.h"

DictionaryWindow::DictionaryWindow(Renderer* renderer, AppModel* appModel, HWND parentWindow, HINSTANCE hInstance, DWORD x, DWORD y)
    : ContentWindow(renderer, appModel, parentWindow, hInstance, x, y, 0, 0)
{
}

void DictionaryWindow::ShowFullTranslation(int dictionaryIndex)
{
    appModel->TranslateWordFromDictionary(dictionaryIndex);
}

POINT DictionaryWindow::RenderDC()
{
    ContentWindow::RenderDC();

    vector<LogRecord> records = appModel->GetDictionaryRecords();

    POINT bottomRight = { 0, 0 };

    int curY = paddingY / 2;

    size_t countToShow = min(200, records.size());

    wstring title = L"Showing " + to_wstring(countToShow) + L" out of " + to_wstring(records.size());
    POINT lineBottomRight = renderer->PrintText(inMemoryDC, title.c_str(), fontItalic, Colors::Gray, paddingX, curY, &bottomRight);
    
    curY = lineBottomRight.y + paddingY / 2;
    
    for (size_t i = 0; i < countToShow; ++i)
    {
        LogRecord record = records[i];
        POINT lineBottomRight = renderer->PrintText(inMemoryDC, record.Word.c_str(), fontNormal, Colors::Black, paddingX * 2 + 4, curY, &bottomRight);
        renderer->PrintText(inMemoryDC, wstring(L" (" + to_wstring(record.Count) + L")").c_str(), fontNormal, Colors::Gray, lineBottomRight.x + 1, curY, &bottomRight);
        
        HoverIconButtonWindow* translateButton = new HoverIconButtonWindow(
            renderer,
            hWindow,
            hInstance,
            paddingX,
            curY + 2,
            13,
            13,
            IDB_TRANSLATE_INACTIVE,
            IDB_TRANSLATE,
            bind(&DictionaryWindow::ShowFullTranslation, this, i));

        AddChildWindow(translateButton);

        curY += lineHeight;
    }
    
    return bottomRight;
}

DictionaryWindow::~DictionaryWindow()
{
}