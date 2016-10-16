#pragma once
#include "View\Framework\Windows\Window.h"
#include "View\Content\Dictionary\DictionaryWindow.h"
#include "View\Content\Translation\TranslationWindow.h"
#include "View\Providers\HotkeyProvider.h"
#include "View\Providers\TrayIconProvider.h"
#include "Controllers\AppController.h"

class MainWindow : public Window
{
private:
    AppController* appController;
    HotkeyProvider* hotkeyProvider;
    TrayIconProvider* trayIconProvider;

    TranslationWindow* translationWindow;
    DictionaryWindow* dictionaryWindow;

    void CreateViews();

    void Scale(double scaleFactorAjustment);
    void Resize() override;
    Window* GetCurrentView() const;

    LRESULT WindowProcedure(UINT message, WPARAM wParam, LPARAM lParam) override;

protected:
    void SpecifyWindowClass(WNDCLASSEX* windowClass) override;
    Size RenderContent(Renderer* renderer) override;

public:
    MainWindow(WindowContext* context, WindowDescriptor descriptor, AppController* appController, HotkeyProvider* hotkeyProvider, TrayIconProvider* trayIconProvider);
    ~MainWindow();

    void Initialize() override;

    void Minimize();
    void Maximize();
    void ShowError(wstring message);
};