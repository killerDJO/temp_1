#include "PrecompiledHeaders\stdafx.h"
#include "Windows\Content\Base\ContentWindow.h"

ContentWindow::ContentWindow(HWND parentWindow, HINSTANCE hInstance, DWORD x, DWORD y, DWORD width, DWORD height)
{
	this->parentWindow = parentWindow;
	this->hInstance = hInstance;
	this->initialX = x;
	this->initialY = y;
	this->width = width;
	this->height = height;

	const TCHAR* className = TEXT("STT_CONTENT");
	WNDCLASSEX wnd;
	if (!GetClassInfoEx(hInstance, className, &wnd))
	{
		wnd = { 0 };
		wnd.hInstance = hInstance;
		wnd.lpszClassName = className;
		wnd.lpfnWndProc = WndProc;
		wnd.cbSize = sizeof (WNDCLASSEX);
		wnd.hCursor = LoadCursor(NULL, IDC_ARROW);
		wnd.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);

		if (!RegisterClassEx(&wnd))
		{
			FatalAppExit(0, TEXT("Couldn't register window class!"));
		}
	}

	this->hWindow = CreateWindowA(
		"STT_CONTENT",
		NULL,
		WS_CHILD | WS_VISIBLE,
		x,
		y,
		width,
		height,
		this->parentWindow,
		NULL,
		this->hInstance,
		this);

	this->InitializeFonts();
	this->InitializeBrushes();
	this->InitializeInMemoryDC();
}

HWND ContentWindow::GetHandle()
{
	return this->hWindow;
}

HINSTANCE ContentWindow::GetInstance()
{
	return this->hInstance;
}

LRESULT CALLBACK ContentWindow::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	ContentWindow* instance = (ContentWindow*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
	
	CREATESTRUCT* createstruct;
	RECT rcWindow;
	POINTS pos;

	switch (message)
	{
	case WM_NCCREATE:
	case WM_CREATE:
		createstruct = (CREATESTRUCT*)lParam;
		SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)createstruct->lpCreateParams);
		return TRUE;

	case WM_MOVE:
		pos = MAKEPOINTS(lParam);
		GetWindowRect(hWnd, &rcWindow);
		MoveWindow(hWnd, pos.x, pos.y, rcWindow.right - rcWindow.left, rcWindow.bottom - rcWindow.top, FALSE);
		InvalidateRect(hWnd, NULL, FALSE);
		return TRUE;
	
	case WM_NCPAINT:
	case WM_PAINT:
		instance->Draw();
		break;

	case WM_SHOWWINDOW:
		instance->Draw();
		break;

	case WM_HSCROLL:
	case WM_VSCROLL:
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

void ContentWindow::InitializeFonts()
{
	HDC hdc = GetDC(hWindow);

	long lfHeight = -MulDiv(LINE_HEIGHT / 2, GetDeviceCaps(hdc, LOGPIXELSY), 72);
	long lfHeightHeader = -MulDiv(int(LINE_HEIGHT * 3 / 5.0), GetDeviceCaps(hdc, LOGPIXELSY), 72);
	long lfHeightSmall = -MulDiv(int(LINE_HEIGHT * 3 / 7.0), GetDeviceCaps(hdc, LOGPIXELSY), 72);

	this->fontNormal = CreateFont(lfHeight, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, TEXT("Arial"));
	this->fontHeader = CreateFont(lfHeightHeader, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, TEXT("Arial"));
	this->fontItalic = CreateFont(lfHeight, 0, 0, 0, 0, TRUE, 0, 0, 0, 0, 0, 0, 0, TEXT("Arial"));
	this->fontSmall = CreateFont(lfHeightSmall, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, TEXT("Arial"));
}

void ContentWindow::InitializeBrushes()
{
	this->grayBrush = CreateSolidBrush(RGB(170, 170, 170));
}

void ContentWindow::InitializeInMemoryDC()
{
	this->inMemoryHDC = CreateCompatibleDC(NULL);

	BITMAPINFO i;
	ZeroMemory(&i.bmiHeader, sizeof(BITMAPINFOHEADER));
	i.bmiHeader.biWidth = width;
	i.bmiHeader.biHeight = height;
	i.bmiHeader.biPlanes = 1;
	i.bmiHeader.biBitCount = 24;
	i.bmiHeader.biSizeImage = 0;
	i.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	i.bmiHeader.biClrUsed = 0;
	i.bmiHeader.biClrImportant = 0;
	VOID *pvBits;
	HBITMAP bitmap = CreateDIBSection(this->inMemoryHDC, &i, DIB_RGB_COLORS, &pvBits, NULL, 0);

	SelectObject(this->inMemoryHDC, bitmap);
}

POINT ContentWindow::RenderResult(TranslateResult translateResult)
{
	this->translateResult = translateResult;
	POINT bottomRight = RenderDC();
	this->ResetWindow(bottomRight);
	InvalidateRect(this->hWindow, NULL, FALSE);
	return bottomRight;
}

void ContentWindow::ResetWindow(POINT bottomRight)
{
	MoveWindow(this->hWindow, this->initialX, this->initialY, bottomRight.x, bottomRight.y, FALSE);
}

POINT ContentWindow::RenderDC()
{
	RECT rect;
	rect.top = 0;
	rect.left = 0;
	rect.bottom = height;
	rect.right = width;
	FillRect(inMemoryHDC, &rect, CreateSolidBrush(RGB(255, 255, 255)));
	return { 0 };
}

void ContentWindow::Draw()
{
	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(this->hWindow, &ps);
	
	DWORD res = BitBlt(hdc, 0, 0, width, height, inMemoryHDC, 0, 0, SRCCOPY);

	EndPaint(this->hWindow, &ps);
}

ContentWindow::~ContentWindow()
{	
	DeleteObject(this->fontNormal);
	DeleteObject(this->fontHeader);
	DeleteObject(this->fontItalic);
	DeleteObject(this->fontSmall);
	DeleteObject(this->grayBrush);

	DeleteDC(this->inMemoryHDC);
}