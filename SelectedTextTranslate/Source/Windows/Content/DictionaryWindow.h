#pragma once
#include "Windows\Content\Base\ContentWindow.h"
#include "Entities\LogRecord.h"
#include "Windows\Buttons\HoverIconButtonWindow.h"

class DictionaryWindow : public ContentWindow
{
protected:
    SIZE RenderDC(Renderer* renderer) override;

private:
    void ShowFullTranslation(int dictionaryIndex);

public:
    DictionaryWindow(HINSTANCE hInstance, RenderingContext* renderingContext, ScrollProvider* scrollProvider, WindowDescriptor descriptor, HWND parentWindow, AppModel* appModel);
    ~DictionaryWindow();
};