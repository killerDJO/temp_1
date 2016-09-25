#pragma once
#include "Windows\Base\Window.h"
#include "Model\AppModel.h"

#define ID_TRAY_APP_ICON    5000
#define WM_TRAYICON ( WM_USER + 1 )

#define ID_TRAY_EXIT_CONTEXT_MENU_ITEM          3000
#define ID_TRAY_TRANSLATE_CONTEXT_MENU_ITEM     3002
#define ID_TRANSLATE_HOTKEY                     3003
#define ID_PLAYTEXT_HOTKEY                      3004
#define ID_TRAY_DICTIONARY_CONTEXT_MENU_ITEM    3005
#define ID_ZOOM_IN_HOTKEY                       3006
#define ID_ZOOM_OUT_HOTKEY                      3007

class MainWindow : public Window
{
private:
    static UINT WM_TASKBARCREATED;

    HMENU menu;
    NOTIFYICONDATA notifyIconData;

    AppModel* appModel;

    void InitNotifyIconData();

    void Scale(double scaleFactorAjustment);
    void Resize();

    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

protected:
    void SpecifyWindowClass(WNDCLASSEX* windowClass) override;
    Size RenderDC(Renderer* renderer) override;

public:
    MainWindow(WindowContext* context, WindowDescriptor descriptor, AppModel* appModel);
    ~MainWindow();

    void Initialize() override;

    void Minimize();
    void Maximize() const;
};