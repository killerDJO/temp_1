#pragma once
#include "Windows\Base\ChildWindow.h"

class HoverButtonWindow : public ChildWindow
{
private:
    function<void()> clickCallback;

    bool isHovered;

    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

protected:
    HDC hoverStateHDC;
    HDC normalStateHDC;

    POINT RenderDC() override;
    virtual void RenderStatesHDC() = 0;

public:
    HoverButtonWindow(HWND parentWindow, HINSTANCE hInstance, DWORD x, DWORD y, DWORD width, DWORD height, function<void()> clickCallback);
    virtual ~HoverButtonWindow();

    void Initialize() override;
};