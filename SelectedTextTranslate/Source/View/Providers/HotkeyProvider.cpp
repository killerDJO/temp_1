#include "View\Providers\HotkeyProvider.h"

HotkeyProvider::HotkeyProvider(HotkeySettings settings, CompositionRoot* root)
{
    this->hotkeysRegistry = vector<HotkeyInfo>();
    this->isSuspended = false;
    this->hotkeyIdToHotkeyMap = map<int, int>();

    SetHotkeysSettings(settings);

    root->GetService<MessageBus>()->OnSuspendHotkeys.Subscribe(bind(&HotkeyProvider::SuspendHotkeys, this));
    root->GetService<MessageBus>()->OnEnableHotkeys.Subscribe(bind(&HotkeyProvider::EnableHotkeys, this));
}

void HotkeyProvider::SetHotkeysSettings(HotkeySettings settings)
{
    hotkeyIdToHotkeyMap[TranslateHotkeyId] = settings.GetTranslateHotkey();
    hotkeyIdToHotkeyMap[PlayTextHotkeyId] = settings.GetPlayTextHotkey();
    hotkeyIdToHotkeyMap[ZoomInHotkeyId] = settings.GetZoomInHotkey();
    hotkeyIdToHotkeyMap[ZoomOutHotkeyId] = settings.GetZoomOutHotkey();

    SuspendHotkeys();
    UpdateHotkeysInfo();
    EnableHotkeys();
}

void HotkeyProvider::RegisterCustomHotkey(HotkeyInfo hotkeyInfo)
{
    if(!isSuspended)
    {
        hotkeyInfo.Register();
    }
    
    hotkeysRegistry.push_back(hotkeyInfo);
}

void HotkeyProvider::UnregisterCustomHotkey(HWND windowHandle, int hotkeyId)
{
    for(size_t i = 0; i < hotkeysRegistry.size(); ++i)
    {
        HotkeyInfo hotkeyInfo = hotkeysRegistry[i];
        if(hotkeyInfo.GetHotkeyId() == hotkeyId && hotkeyInfo.GetWindowHandle() == windowHandle)
        {
            hotkeyInfo.Unregister();
            hotkeysRegistry.erase(hotkeysRegistry.begin() + i);
            break;
        }
    }
}

void HotkeyProvider::RegisterTranslateHotkey(HWND windowHandle, function<void()> pressedCallback)
{
    RegisterCustomHotkey(HotkeyInfo(windowHandle, pressedCallback, TranslateHotkeyId, hotkeyIdToHotkeyMap[TranslateHotkeyId]));
}

void HotkeyProvider::RegisterPlayTextHotkey(HWND windowHandle, function<void()> pressedCallback)
{
    RegisterCustomHotkey(HotkeyInfo(windowHandle, pressedCallback, PlayTextHotkeyId, hotkeyIdToHotkeyMap[PlayTextHotkeyId]));
}

void HotkeyProvider::RegisterZoomInHotkey(HWND windowHandle, function<void()> pressedCallback)
{
    RegisterCustomHotkey(HotkeyInfo(windowHandle, pressedCallback, ZoomInHotkeyId, hotkeyIdToHotkeyMap[ZoomInHotkeyId]));
}

void HotkeyProvider::RegisterZoomOutHotkey(HWND windowHandle, function<void()> pressedCallback)
{
    RegisterCustomHotkey(HotkeyInfo(windowHandle, pressedCallback, ZoomOutHotkeyId, hotkeyIdToHotkeyMap[ZoomOutHotkeyId]));
}

void HotkeyProvider::UnregisterZoomInHotkey(HWND windowHandle)
{
    UnregisterCustomHotkey(windowHandle, ZoomInHotkeyId);
}

void HotkeyProvider::UnregisterZoomOutHotkey(HWND windowHandle)
{
    UnregisterCustomHotkey(windowHandle, ZoomOutHotkeyId);
}

void HotkeyProvider::UnregisterTranslateHotkey(HWND windowHandle)
{
    UnregisterCustomHotkey(windowHandle, TranslateHotkeyId);
}

void HotkeyProvider::UnregisterPlayTextHotkey(HWND windowHandle)
{
    UnregisterCustomHotkey(windowHandle, PlayTextHotkeyId);
}

void HotkeyProvider::SuspendHotkeys()
{
    isSuspended = true;
    for (size_t i = 0; i < hotkeysRegistry.size(); ++i)
    {
        hotkeysRegistry[i].Unregister();
    }
}

void HotkeyProvider::EnableHotkeys()
{
    isSuspended = false;
    for (size_t i = 0; i < hotkeysRegistry.size(); ++i)
    {
        hotkeysRegistry[i].Register();
    }
}

void HotkeyProvider::UpdateHotkeysInfo()
{
    for (size_t i = 0; i < hotkeysRegistry.size(); ++i)
    {
        hotkeysRegistry[i].UpdateHotkey(hotkeyIdToHotkeyMap[hotkeysRegistry[i].GetHotkeyId()]);
    }
}

void HotkeyProvider::ProcessHotkey(DWORD hotkeyId)
{
    for (size_t i = 0; i < hotkeysRegistry.size(); ++i)
    {
        if (hotkeysRegistry[i].GetHotkeyId() == hotkeyId)
        {
            hotkeysRegistry[i].GetPressedCallback()();
        }
    }
}