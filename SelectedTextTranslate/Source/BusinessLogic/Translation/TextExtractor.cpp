#include "BusinessLogic\Translation\TextExtractor.h"
#include "Infrastructure\ErrorHandling\ExceptionHelper.h"

TextExtractor::TextExtractor()
{
}

wstring TextExtractor::GetSelectedText() const
{
    wstring result = L"";

    int key_count = 4;
    INPUT* input = new INPUT[key_count];
    for( int i = 0; i < key_count; i++ )
    {
        input[i].ki.dwFlags = 0;
        input[i].type = INPUT_KEYBOARD;
    }

    input[0].ki.wVk = VK_CONTROL;
    input[0].ki.wScan = MapVirtualKey( VK_CONTROL, MAPVK_VK_TO_VSC );
    input[1].ki.wVk = 0x43; 
    input[1].ki.wScan = MapVirtualKey( 0x43, MAPVK_VK_TO_VSC );
    input[2].ki.dwFlags = KEYEVENTF_KEYUP;
    input[2].ki.wVk = input[0].ki.wVk;
    input[2].ki.wScan = input[0].ki.wScan;
    input[3].ki.dwFlags = KEYEVENTF_KEYUP;
    input[3].ki.wVk = input[1].ki.wVk;
    input[3].ki.wScan = input[1].ki.wScan;

    AssertWinApiResult(SendInput(key_count, LPINPUT(input), sizeof(INPUT)));

    // Waiting for input event
    Sleep(100);

    AssertWinApiResult(OpenClipboard(nullptr));

    HGLOBAL hglb = GetClipboardData(CF_UNICODETEXT);

    AssertWinApiResult(CloseClipboard());

    if(hglb == nullptr)
    {
        return wstring();
    }

    TCHAR* lpstr = static_cast<TCHAR*>(GlobalLock(hglb));
    AssertWinApiResult(lpstr);

    if(lpstr)
    {
        result = wstring(lpstr);
    }

    AssertWinApiResult(GlobalUnlock(hglb));

    return result;
}