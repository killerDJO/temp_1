#include "Presentation\Components\Settings\Hotkeys\HotkeySettingsView.h"
#include "Presentation\Framework\Rendering\Dto\RenderResult.h"

HotkeySettingsView::HotkeySettingsView(CommonContext* context, View* parentView, ModelHolder<SettingsGroupViewModel<HotkeySettings>*>* modelHolder)
    : SettingsGroupView(context, parentView, modelHolder)
{
    this->font = RenderingProvider->CreateCustomFont(FontSizes::Medium);
    this->controlsMargin = ScaleProvider->Scale(7);

    this->Name = L"HotkeySettingsWindow";
}

void HotkeySettingsView::RenderSettingsContent(RenderDescriptor renderDescriptor, HotkeySettings* settings)
{
    RenderPosition renderPosition = renderDescriptor.GetRenderPosition().MoveY(PaddingY);
    Renderer* renderer = renderDescriptor.GetRenderer();

    hotkeyInputWindows.clear();

    renderPosition = RenderHotkeyEditControl(
        settings,
        RenderDescriptor(renderer, renderPosition),
        L"Translate Selected Text:",
        settings->GetTranslateHotkey(),
        [this](HotkeySettings* hotkeySettings, DWORD hotkey) -> void
    {
        hotkeySettings->SetTranslateHotkey(hotkey);
    });

    renderPosition = RenderHotkeyEditControl(
        settings,
        RenderDescriptor(renderer, renderPosition.MoveY(controlsMargin)),
        L"Play Selected Text:",
        settings->GetPlayTextHotkey(),
        [this](HotkeySettings* hotkeySettings, DWORD hotkey) -> void
    {
        hotkeySettings->SetPlayTextHotkey(hotkey);
    });

    renderPosition = RenderHotkeyEditControl(
        settings,
        RenderDescriptor(renderer, renderPosition.MoveY(controlsMargin)),
        L"Zoom In:",
        settings->GetZoomInHotkey(),
        [this](HotkeySettings* hotkeySettings, DWORD hotkey) -> void
    {
        hotkeySettings->SetZoomInHotkey(hotkey);
    });

    RenderHotkeyEditControl(
        settings,
        RenderDescriptor(renderer, renderPosition.MoveY(controlsMargin)),
        L"Zoom Out:",
        settings->GetZoomOutHotkey(),
        [this](HotkeySettings* hotkeySettings, DWORD hotkey) -> void
    {
        hotkeySettings->SetZoomOutHotkey(hotkey);
    });

    SetHotkeyInputsValiationState();
    renderer->IncreaseHeight(2 * PaddingY);
}

RenderResult HotkeySettingsView::RenderHotkeyEditControl(
    HotkeySettings* settings,
    RenderDescriptor renderDescriptor,
    wstring title,
    int hotkey,
    function<void(HotkeySettings*, DWORD)> hotkeySetter)
{
    RenderPosition renderPosition = renderDescriptor.GetRenderPosition();

    TextRenderResult textRenderResult = renderDescriptor.GetRenderer()->PrintText(
        title.c_str(),
        font,
        Colors::Black,
        renderPosition.MoveY(font->GetAscent()));

    renderPosition = renderPosition
        .SetY(textRenderResult.GetBottomY())
        .MoveY(1);

    HotKeyInputControl* hotKeyInputWindow = new HotKeyInputControl(Context, this);
    hotKeyInputWindow->SetPosition(renderPosition.GetPosition());
    hotKeyInputWindow->SetHotkey(hotkey);
    hotKeyInputWindow->OnHotkeyChanged.Subscribe([hotkeySetter, settings, this](DWORD newHotkey)
    {
        hotkeySetter(settings, newHotkey);
        SetHotkeyInputsValiationState();
        OnSettingsChanged.Notify();
    });
    hotKeyInputWindow->InitializeAndRender();
    hotkeyInputWindows.push_back(hotKeyInputWindow);

    renderDescriptor.GetRenderer()->UpdateRenderedContentSize(hotKeyInputWindow);

    return RenderResult(renderPosition.MoveY(hotKeyInputWindow->GetSize().GetHeight()));
}

void HotkeySettingsView::SetHotkeyInputsValiationState()
{
    for (size_t i = 0; i < hotkeyInputWindows.size(); ++i)
    {
        hotkeyInputWindows[i]->MakeValid();
    }

    for (size_t i = 0; i < hotkeyInputWindows.size(); ++i)
    {
        for (size_t j = i + 1; j < hotkeyInputWindows.size(); ++j)
        {
            if (hotkeyInputWindows[i]->GetHotKey() == hotkeyInputWindows[j]->GetHotKey())
            {
                hotkeyInputWindows[i]->MakeInvalid();
                hotkeyInputWindows[j]->MakeInvalid();
            }
        }

        hotkeyInputWindows[i]->Render();
    }
}

HotkeySettingsView::~HotkeySettingsView()
{
    delete font;
}