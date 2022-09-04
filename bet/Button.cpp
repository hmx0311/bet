#include "framework.h"
#include "Button.h"

#include "common.h"
#include "bet.h"

#define BUTTON_ANIMATION_DURATION_SHORT 150
#define BUTTON_ANIMATION_DURATION_LONG  200

WNDPROC defButtonProc;
HTHEME hButtonTheme;

LRESULT CALLBACK buttonProc(HWND hButton, UINT message, WPARAM wParam, LPARAM lParam)
{
	Button* button = (Button*)GetWindowLongPtr(hButton, GWLP_USERDATA);
	LRESULT result = (LRESULT)FALSE;
	if (button != nullptr)
	{
		result = button->wndProc(message, wParam, lParam);
	}
	if (result)
	{
		return result;
	}
	return CallWindowProc(defButtonProc, hButton, message, wParam, lParam);
}

LRESULT Button::wndProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_ERASEBKGND:
		return LRESULT(TRUE);
	case WM_MOVE:
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
	return LRESULT(FALSE);
}

void Button::drawItem(HDC hDC, UINT itemState, RECT& rcItem)
{
	PUSHBUTTONSTATES state = PBS_NORMAL;
	if (itemState & ODS_SELECTED)
	{
		state = PBS_PRESSED;
	}
	else if (itemState & ODS_HOTLIGHT || isTracking)
	{
		state = PBS_HOT;
	}
	if (lastState == state)
	{
		int width = rcItem.right - rcItem.left;
		int height = rcItem.bottom - rcItem.top;
		HBITMAP hBmBuffer = CreateCompatibleBitmap(hDC, width, height);
		HDC hDCMem = CreateCompatibleDC(hDC);
		SelectObject(hDCMem, hBmBuffer);
		SelectObject(hDCMem, hFont);
		drawButton(hDCMem, state, rcItem);
		BitBlt(hDC, rcItem.left, rcItem.top, width, height, hDCMem, 0, 0, SRCCOPY);
		DeleteDC(hDCMem);
		DeleteObject(hBmBuffer);
		return;
	}
	BP_ANIMATIONPARAMS animParams = { sizeof(BP_ANIMATIONPARAMS),0, BPAS_LINEAR, state == PBS_PRESSED ? BUTTON_ANIMATION_DURATION_SHORT : BUTTON_ANIMATION_DURATION_LONG };
	HDC hdcFrom, hdcTo;
	HANIMATIONBUFFER hbpAnimation = BeginBufferedAnimation(hButton, hDC, &rcItem, BPBF_COMPATIBLEBITMAP, NULL, &animParams, &hdcFrom, &hdcTo);
	SelectObject(hdcFrom, hFont);
	SelectObject(hdcTo, hFont);
	drawButton(hdcFrom, lastState, rcItem);
	drawButton(hdcTo, state, rcItem);
	BufferedPaintStopAllAnimations(hButton);
	EndBufferedAnimation(hbpAnimation, TRUE);
	lastState = state;
}

void Button::attach(HWND hButton)
{
	this->hButton = hButton;
	SetWindowLongPtr(hButton, GWLP_USERDATA, (LONG_PTR)this);
	SetWindowLongPtr(hButton, GWLP_WNDPROC, (LONG_PTR)buttonProc);
}

HWND Button::getHwnd()
{
	return hButton;
}

void Button::setText(LPCWSTR str)
{
	SetWindowText(hButton, str);
}

void Button::setIcon(HICON hIcon)
{
	this->hIcon = hIcon;
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
	if (GetWindowLongPtr(hButton, GWL_STYLE) & BS_FLAT)
	{
		if (state == PBS_HOT)
		{
			FillRect(hDC, &rcItem, (HBRUSH)GetStockObject(LTGRAY_BRUSH));
		}
		rcContent.left++;
		rcContent.top++;
		rcContent.right--;
		rcContent.bottom--;
	}
	else
	{
		DrawThemeBackground(hButtonTheme, hDC, BP_PUSHBUTTON, state, &rcItem, 0);
		GetThemeBackgroundContentRect(hButtonTheme, hDC, BP_PUSHBUTTON, state, &rcItem, &rcContent);
	}

	if (hIcon != nullptr)
	{
		ICONINFO iconInfo;
		GetIconInfo(hIcon, &iconInfo);
		BITMAP bmMask;
		GetObject(iconInfo.hbmColor, sizeof(BITMAP), &bmMask);
		HDC hDCImage = CreateCompatibleDC(hDC);
		HBITMAP hBmBuffer = CreateCompatibleBitmap(hDC, bmMask.bmWidth, bmMask.bmHeight);
		SelectObject(hDCImage, hBmBuffer);
		SetStretchBltMode(hDCImage, HALFTONE);
		StretchBlt(hDCImage, 0, 0, bmMask.bmWidth, bmMask.bmHeight, hDC, rcContent.left, rcContent.top, rcContent.right - rcContent.left, rcContent.bottom - rcContent.top, SRCCOPY);
		DrawIcon(hDCImage, 0, 0, hIcon);
		SetStretchBltMode(hDC, HALFTONE);
		StretchBlt(hDC, rcContent.left, rcContent.top, rcContent.right - rcContent.left, rcContent.bottom - rcContent.top, hDCImage, 0, 0, bmMask.bmWidth, bmMask.bmHeight, SRCCOPY);
		DeleteObject(hBmBuffer);
		DeleteObject(hDCImage);
	}
	SetBkMode(hDC, TRANSPARENT);
	TCHAR str[10];
	GetWindowText(hButton, str, 10);
	DrawText(hDC, str, wcslen(str), &rcContent, DT_SINGLELINE | DT_VCENTER | DT_CENTER);
}
