#pragma once
#include "View\Framework\RenderingContext.h"
#include "View\Framework\DeviceContextBuffer.h"
#include "View\Framework\Providers\DeviceContextProvider.h"

class RenderingContext;
class ScrollProvider;
class Window;

class Renderer
{
private:
    RenderingContext* renderingContext;
    ScaleProvider* scaleProvider;
    ScrollProvider* scrollProvider;

    Size originalSize;

    HBRUSH backgroundBrush;

    vector<function<void(HDC)>> renderActions;

    void ClearDeviceContext(HDC deviceContext, Size deviceContextSize) const;

public:
    Renderer(RenderingContext* renderingContext, DeviceContextProvider* deviceContextProvider, ScaleProvider* scaleProvider);
    ~Renderer();

    Point PrintText(const wstring text, HFONT font, Colors color, Point position, DWORD horizontalAlignment = TA_LEFT);
    void DrawRect(Rect rect, HBRUSH brush);
    void DrawBorderedRect(Rect rect, HBRUSH brush, int borderWidth, Colors borderColor);
    void SetBackground(HBRUSH backgroundBrush);

    int GetFontAscent(HFONT font) const;
    int GetFontDescent(HFONT font) const;
    int GetFontStrokeHeight(HFONT font) const;
    int GetFontHeight(HFONT font) const;

    Size GetScaledSize() const;
    Size GetSize() const;

    void IncreaseWidth(int widthToAdd);
    void IncreaseHeight(int heightToAdd);
    void UpdateRenderedContentSize(Window* window);

    void Render(HDC deviceContext, Size deviceContextSize);
    void Render(DeviceContextBuffer* deviceContextBuffer);
};