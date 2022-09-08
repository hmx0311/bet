#include "framework.h"
#include "button.h"

#include "common.h"
#include "bet.h"

#define BUTTON_ANIMATION_DURATION_SHORT 150
#define BUTTON_ANIMATION_DURATION_LONG  200

#define BUTTON_MARGIN_RATIO 0.1f
#define PRESSED_SQUEEZE 0.0625f

HTHEME hButtonTheme;

LRESULT CALLBACK buttonSubclassProc(HWND hButton, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
	Button* button = (Button*)GetWindowLongPtr(hButton, GWLP_USERDATA);
	if (button != nullptr)
	{
		return button->wndProc(msg, wParam, lParam);
	}
	if (msg == WM_UPDATEUISTATE)
	{
		wParam &= ~MAKELONG(0, UISF_HIDEFOCUS | UISF_ACTIVE);
	}
	return DefSubclassProc(hButton, msg, wParam, lParam);
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
		return LRESULT(TRUE);
	case WM_MOVE:
	case WM_SHOWWINDOW:
		BufferedPaintStopAllAnimations(hButton);
		break;
	case WM_MOUSEMOVE:
		{
			if (isTracking)
			{
				break;
			}
			isTracking = true;
			TRACKMOUSEEVENT eventTrack = { sizeof(TRACKMOUSEEVENT),TME_LEAVE,hButton ,0 };
			TrackMouseEvent(&eventTrack);
			InvalidateRect(hButton, nullptr, FALSE);
			break;
		}
	case WM_MOUSELEAVE:
		{
			isTracking = false;
			InvalidateRect(hButton, nullptr, FALSE);
			break;
		}
	case WM_LBUTTONDBLCLK:
		SendMessage(hButton, WM_LBUTTONDOWN, wParam, lParam);
		return LRESULT(TRUE);
	}
	return DefSubclassProc(hButton, msg, wParam, lParam);
}

void Button::drawItem(HDC hDC, UINT itemState, RECT& rcItem)
{
	if (hButtonTheme == nullptr)
	{
		HBITMAP hBmBuffer = CreateCompatibleBitmap(hDC, rcItem.right - rcItem.left, rcItem.bottom - rcItem.top);
		HDC hDCMem = CreateCompatibleDC(hDC);
		SelectObject(hDCMem, hBmBuffer);
		FillRect(hDCMem, &rcItem, hBkgBrush);
		RECT rcContent = rcItem;
		int margin = BUTTON_MARGIN_RATIO * min(rcContent.right - rcContent.left, rcContent.bottom - rcContent.top);
		if (!(GetWindowLongPtr(hButton, GWL_STYLE) & BS_FLAT))
		{
			margin++;
			UINT uStyle = DFCS_BUTTONPUSH;
			if (itemState & ODS_SELECTED)
			{
				uStyle |= DFCS_PUSHED;
				rcContent.left++;
				rcContent.top++;
				rcContent.right++;
				rcContent.bottom++;
			}
			DrawFrameControl(hDCMem, &rcItem, DFC_BUTTON, uStyle);
		}
		rcContent.left += margin;
		rcContent.top += margin;
		rcContent.right -= margin;
		rcContent.bottom -= margin;
		if (hIcon != nullptr)
		{
			HDC hDCImage = CreateCompatibleDC(hDCMem);
			int xSqueeze = 0, ySqueeze = 0;
			if (GetWindowLongPtr(hButton, GWL_STYLE) & BS_FLAT&& itemState & ODS_SELECTED)
			{
				xSqueeze = PRESSED_SQUEEZE * iconWidth;
				ySqueeze = PRESSED_SQUEEZE * iconHeight;
			}
			HBITMAP hBmBuffer = CreateCompatibleBitmap(hDCMem, iconWidth + 2 * xSqueeze, iconHeight + 2 * ySqueeze);
			SelectObject(hDCImage, hBmBuffer);
			SetStretchBltMode(hDCImage, HALFTONE);
			StretchBlt(hDCImage, 0, 0, iconWidth + 2 * xSqueeze, iconHeight + 2 * ySqueeze,
				hDCMem, rcContent.left, rcContent.top, rcContent.right - rcContent.left, rcContent.bottom - rcContent.top, SRCCOPY);
			DrawIcon(hDCImage, xSqueeze, ySqueeze, hIcon);
			SetStretchBltMode(hDCMem, HALFTONE);
			StretchBlt(hDCMem, rcContent.left, rcContent.top, rcContent.right - rcContent.left, rcContent.bottom - rcContent.top,
				hDCImage, 0, 0, iconWidth + 2 * xSqueeze, iconHeight + 2 * ySqueeze, SRCCOPY);
			DeleteObject(hBmBuffer);
			DeleteObject(hDCImage);
		}
		SelectObject(hDCMem, (HFONT)SendMessage(hButton, WM_GETFONT, 0, 0));
		SetBkMode(hDCMem, TRANSPARENT);
		SetTextColor(hDCMem, GetSysColor(COLOR_BTNTEXT));
		TCHAR str[10];
		GetWindowText(hButton, str, 10);
		DrawText(hDCMem, str, wcslen(str), &rcContent, DT_SINGLELINE | DT_VCENTER | DT_CENTER);
		BitBlt(hDC, rcItem.left, rcItem.top, rcItem.right - rcItem.left, rcItem.bottom - rcItem.top, hDCMem, 0, 0, SRCCOPY);
		DeleteDC(hDCMem);
		DeleteObject(hBmBuffer);
		return;
	}
	PUSHBUTTONSTATES state = PBS_NORMAL;
	if (itemState & ODS_SELECTED)
	{
		state = PBS_PRESSED;
	}
	else if (isTracking)
	{
		state = PBS_HOT;
	}
	if (lastState == state)
	{
		HBITMAP hBmBuffer = CreateCompatibleBitmap(hDC, rcItem.right - rcItem.left, rcItem.bottom - rcItem.top);
		HDC hDCMem = CreateCompatibleDC(hDC);
		SelectObject(hDCMem, hBmBuffer);
		drawButton(hDCMem, state, rcItem);
		BitBlt(hDC, rcItem.left, rcItem.top, rcItem.right - rcItem.left, rcItem.bottom - rcItem.top, hDCMem, 0, 0, SRCCOPY);
		DeleteDC(hDCMem);
		DeleteObject(hBmBuffer);
		return;
	}
	BP_ANIMATIONPARAMS animParams = { sizeof(BP_ANIMATIONPARAMS),0, BPAS_LINEAR, state == PBS_PRESSED ? BUTTON_ANIMATION_DURATION_SHORT : BUTTON_ANIMATION_DURATION_LONG };
	HDC hdcFrom, hdcTo;
	HANIMATIONBUFFER hbpAnimation = BeginBufferedAnimation(hButton, hDC, &rcItem, BPBF_COMPATIBLEBITMAP, nullptr, &animParams, &hdcFrom, &hdcTo);
	if (hdcFrom != nullptr)
	{
		drawButton(hdcFrom, lastState, rcItem);
	}
	drawButton(hdcTo, state, rcItem);
	EndBufferedAnimation(hbpAnimation, TRUE);
	lastState = state;
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
	this->hIcon = hIcon;
	if (hIcon != nullptr)
	{
		ICONINFO iconInfo;
		GetIconInfo(hIcon, &iconInfo);
		BITMAP bmMask;
		GetObject(iconInfo.hbmColor, sizeof(BITMAP), &bmMask);
		iconWidth = bmMask.bmWidth;
		iconHeight = bmMask.bmHeight;
	}
	InvalidateRect(hButton, nullptr, FALSE);
}

void Button::setBkgBrush(HBRUSH hBkgBrush)
{
	this->hBkgBrush = hBkgBrush;
}

void Button::drawButton(HDC hDC, PUSHBUTTONSTATES state, RECT& rcItem)
{
	FillRect(hDC, &rcItem, hBkgBrush);
	RECT rcContent = rcItem;
	int margin;
	if (GetWindowLongPtr(hButton, GWL_STYLE) & BS_FLAT)
	{
		margin = BUTTON_MARGIN_RATIO * min(rcContent.right - rcContent.left, rcContent.bottom - rcContent.top);
		if (state == PBS_HOT || state == PBS_PRESSED)
		{
			HGDIOBJ oldBrush = SelectObject(hDC, GetStockObject(LTGRAY_BRUSH));
			HGDIOBJ oldPen = SelectObject(hDC, GetStockObject(NULL_PEN));
			RoundRect(hDC, rcItem.left, rcItem.top, rcItem.right + 1, rcItem.bottom + 1, 2 * margin, 2 * margin);
			SelectObject(hDC, oldBrush);
			SelectObject(hDC, oldPen);
		}
	}
	else
	{
		HRESULT hr = DrawThemeBackground(hButtonTheme, hDC, BP_PUSHBUTTON, state, &rcItem, 0);
		if (!SUCCEEDED(hr))
		{
			TCHAR str[100];
			_stprintf(str, _T("hTheme=%p,error code=%x,reopen themedata?"), hButtonTheme, hr);
			//if (MessageBox(hButton, str, _T("!"), MB_YESNO) == IDYES)
			{
				CloseThemeData(hButtonTheme);
				hButtonTheme = OpenThemeData(hButton, _T("Button"));
			}
			_stprintf(str, _T("hTheme=%p"), hButtonTheme);
			//MessageBox(hButton, str, _T("!"), MB_OK);
		}
		GetThemeBackgroundContentRect(hButtonTheme, hDC, BP_PUSHBUTTON, state, &rcItem, &rcContent);
		margin = BUTTON_MARGIN_RATIO * min(rcContent.right - rcContent.left, rcContent.bottom - rcContent.top) - 1;
	}
	rcContent.left += margin;
	rcContent.top += margin;
	rcContent.right -= margin;
	rcContent.bottom -= margin;

	if (hIcon != nullptr)
	{
		HDC hDCImage = CreateCompatibleDC(hDC);
		int xSqueeze = 0, ySqueeze = 0;
		if (state == PBS_PRESSED)
		{
			xSqueeze = PRESSED_SQUEEZE * iconWidth;
			ySqueeze = PRESSED_SQUEEZE * iconHeight;
		}
		HBITMAP hBmBuffer = CreateCompatibleBitmap(hDC, iconWidth + 2 * xSqueeze, iconHeight + 2 * ySqueeze);
		SelectObject(hDCImage, hBmBuffer);
		SetStretchBltMode(hDCImage, HALFTONE);
		StretchBlt(hDCImage, 0, 0, iconWidth + 2 * xSqueeze, iconHeight + 2 * ySqueeze,
			hDC, rcContent.left, rcContent.top, rcContent.right - rcContent.left, rcContent.bottom - rcContent.top, SRCCOPY);
		DrawIcon(hDCImage, xSqueeze, ySqueeze, hIcon);
		SetStretchBltMode(hDC, HALFTONE);
		StretchBlt(hDC, rcContent.left, rcContent.top, rcContent.right - rcContent.left, rcContent.bottom - rcContent.top,
			hDCImage, 0, 0, iconWidth + 2 * xSqueeze, iconHeight + 2 * ySqueeze, SRCCOPY);
		DeleteObject(hBmBuffer);
		DeleteObject(hDCImage);
	}
	SelectObject(hDC, (HFONT)SendMessage(hButton, WM_GETFONT, 0, 0));
	SetBkMode(hDC, TRANSPARENT);
	TCHAR str[10];
	GetWindowText(hButton, str, 10);
	DrawText(hDC, str, wcslen(str), &rcContent, DT_SINGLELINE | DT_VCENTER | DT_CENTER);
}
