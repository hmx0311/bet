#include "framework.h"
#include "button.h"

#include <windowsx.h>

#define BUTTON_ANIMATION_DURATION_SHORT 167
#define BUTTON_ANIMATION_DURATION_LONG  200

#define BUTTON_MARGIN_RATIO 0.08f
#define PRESSED_SQUEEZE 0.03125f

HTHEME hButtonTheme;

LRESULT CALLBACK buttonSubclassProc(HWND hButton, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
	switch (msg)
	{
	case WM_UPDATEUISTATE:
		wParam &= ~MAKELONG(0, UISF_HIDEFOCUS);
		break;
	case WM_SETFOCUS:
		if (HIBYTE(GetKeyState(VK_TAB)))
		{
			SetFocus(GetNextDlgTabItem(GetParent(hButton), hButton, HIBYTE(GetKeyState(VK_SHIFT))));
		}
		else if (IsWindowVisible((HWND)wParam) && !GetWindowSubclass((HWND)wParam, buttonSubclassProc, 0, 0))
		{
			SetFocus((HWND)wParam);
		}
		else
		{
			SetFocus(GetNextDlgTabItem(GetParent(hButton), hButton, FALSE));
		}
		return 0;
	case WM_KILLFOCUS:
		return 0;
	}
	Button* button = (Button*)GetWindowLongPtr(hButton, GWLP_USERDATA);
	return button == nullptr ? DefSubclassProc(hButton, msg, wParam, lParam) : button->wndProc(msg, wParam, lParam);
}


//class Button

void Button::attach(HWND hButton)
{
	this->hButton = hButton;
	SetWindowLongPtr(hButton, GWLP_USERDATA, (LONG_PTR)this);
	SetWindowSubclass(hButton, buttonSubclassProc, 0, 0);
}

LRESULT Button::wndProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_ERASEBKGND:
		return (LRESULT)TRUE;
	case WM_MOVE:
	case WM_SHOWWINDOW:
		lastState = PBS_NORMAL;
		BufferedPaintStopAllAnimations(hButton);
		DefSubclassProc(hButton, WM_KILLFOCUS, 0, 0);
		break;
	case WM_PAINT:
		{
			PAINTSTRUCT paintStruct;
			HDC hDC = BeginPaint(hButton, &paintStruct);
			if (!BufferedPaintRenderAnimation(hButton, hDC))
			{
				RECT rcItem;
				GetClientRect(hButton, &rcItem);
				PUSHBUTTONSTATES state = PBS_NORMAL;
				int bst = Button_GetState(hButton);
				if (bst & BST_PUSHED)
				{
					state = PBS_PRESSED;
				}
				else if (bst & BST_HOT)
				{
					state = PBS_HOT;
				}
				if (lastState == state || hButtonTheme == nullptr)
				{
					HDC hMemDC;
					HPAINTBUFFER hPaintBuffer = BeginBufferedPaint(hDC, &rcItem, BPBF_COMPATIBLEBITMAP, nullptr, &hMemDC);
					drawButton(hMemDC, state, rcItem);
					EndBufferedPaint(hPaintBuffer, TRUE);
				}
				else
				{
					BP_ANIMATIONPARAMS animParams = { sizeof(BP_ANIMATIONPARAMS),0, BPAS_LINEAR, state == PBS_PRESSED ? BUTTON_ANIMATION_DURATION_SHORT : BUTTON_ANIMATION_DURATION_LONG };
					HDC hDCFrom, hDCTo;
					HANIMATIONBUFFER hbpAnimation = BeginBufferedAnimation(hButton, hDC, &rcItem, BPBF_COMPATIBLEBITMAP, nullptr, &animParams, &hDCFrom, &hDCTo);
					if (hDCFrom != nullptr)
					{
						drawButton(hDCFrom, lastState, rcItem);
					}
					drawButton(hDCTo, state, rcItem);
					BufferedPaintStopAllAnimations(hButton);
					EndBufferedAnimation(hbpAnimation, TRUE);
					lastState = state;
				}
			}
			EndPaint(hButton, &paintStruct);
			return 0;
		}
	}
	return DefSubclassProc(hButton, msg, wParam, lParam);
}

HWND Button::getHwnd()
{
	return hButton;
}

void Button::setText(PCTSTR str)
{
	SetWindowText(hButton, str);
}

void Button::setIcon(HICON hIcon)
{
	SendMessage(hButton, BM_SETIMAGE, IMAGE_ICON, (WPARAM)hIcon);
	if (hIcon != nullptr)
	{
		ICONINFO iconInfo;
		GetIconInfo(hIcon, &iconInfo);
		BITMAP bmMask;
		GetObject(iconInfo.hbmColor, sizeof(BITMAP), &bmMask);
		iconWidth = bmMask.bmWidth;
		iconHeight = bmMask.bmHeight;
	}
}

void Button::setBkgBrush(HBRUSH hBkgBrush)
{
	this->hBkgBrush = hBkgBrush;
}

void Button::drawButton(HDC hDC, PUSHBUTTONSTATES state, RECT& rcItem)
{
	FillRect(hDC, &rcItem, hBkgBrush);
	RECT rcContent = rcItem;
	int padding;
	if (GetWindowStyle(hButton) & BS_FLAT)
	{
		padding = BUTTON_MARGIN_RATIO * min(rcContent.right - rcContent.left, rcContent.bottom - rcContent.top);
		if (state == PBS_HOT || state == PBS_PRESSED)
		{
			HGDIOBJ oldBrush = SelectObject(hDC, GetStockObject(LTGRAY_BRUSH));
			HGDIOBJ oldPen = SelectObject(hDC, GetStockObject(NULL_PEN));
			RoundRect(hDC, rcItem.left, rcItem.top, rcItem.right + 1, rcItem.bottom + 1, 2 * padding, 2 * padding);
			SelectObject(hDC, oldBrush);
			SelectObject(hDC, oldPen);
		}
	}
	else
	{
		padding = BUTTON_MARGIN_RATIO * min(rcContent.right - rcContent.left, rcContent.bottom - rcContent.top) - 1;
		if (hButtonTheme == nullptr)
		{
			UINT uStyle = DFCS_BUTTONPUSH;
			if (state == PBS_PRESSED)
			{
				uStyle |= DFCS_PUSHED;
				rcContent.left++;
				rcContent.top++;
				rcContent.right++;
				rcContent.bottom++;
			}
			DrawFrameControl(hDC, &rcItem, DFC_BUTTON, uStyle);
			padding += 2;
		}
		else
		{
			DrawThemeBackground(hButtonTheme, hDC, BP_PUSHBUTTON, state, &rcItem, nullptr);
			GetThemeBackgroundContentRect(hButtonTheme, hDC, BP_PUSHBUTTON, state, &rcItem, &rcContent);
		}
	}
	rcContent.left += padding;
	rcContent.top += padding;
	rcContent.right -= padding;
	rcContent.bottom -= padding;

	HICON hIcon = (HICON)SendMessage(hButton, BM_GETIMAGE, IMAGE_ICON, 0);
	if (hIcon != nullptr)
	{
		HDC hImageDC = CreateCompatibleDC(hDC);
		int xSqueeze = 0, ySqueeze = 0;
		if (state == PBS_PRESSED && (hButtonTheme != nullptr || GetWindowStyle(hButton) & BS_FLAT))
		{
			xSqueeze = PRESSED_SQUEEZE * iconWidth + 1;
			ySqueeze = PRESSED_SQUEEZE * iconHeight + 1;
		}
		HBITMAP hBufferBm = CreateCompatibleBitmap(hDC, iconWidth + 2 * xSqueeze, iconHeight + 2 * ySqueeze);
		HGDIOBJ hOldBm = SelectObject(hImageDC, hBufferBm);
		SetStretchBltMode(hImageDC, HALFTONE);
		StretchBlt(hImageDC, 0, 0, iconWidth + 2 * xSqueeze, iconHeight + 2 * ySqueeze,
			hDC, rcContent.left, rcContent.top, rcContent.right - rcContent.left, rcContent.bottom - rcContent.top, SRCCOPY);
		DrawIconEx(hImageDC, xSqueeze, ySqueeze, hIcon, 0, 0, 0, nullptr, DI_NORMAL);
		SetStretchBltMode(hDC, HALFTONE);
		StretchBlt(hDC, rcContent.left, rcContent.top, rcContent.right - rcContent.left, rcContent.bottom - rcContent.top,
			hImageDC, 0, 0, iconWidth + 2 * xSqueeze, iconHeight + 2 * ySqueeze, SRCCOPY);
		SelectObject(hImageDC, hOldBm);
		DeleteDC(hImageDC);
		DeleteObject(hBufferBm);
	}
	HFONT hOldFont = SelectFont(hDC, GetWindowFont(hButton));
	SetBkMode(hDC, TRANSPARENT);
	SetTextColor(hDC, GetSysColor(COLOR_BTNTEXT));
	TCHAR str[10];
	GetWindowText(hButton, str, 10);
	DrawText(hDC, str, wcslen(str), &rcContent, DT_SINGLELINE | DT_VCENTER | DT_CENTER);
	SelectFont(hDC, hOldFont);
}