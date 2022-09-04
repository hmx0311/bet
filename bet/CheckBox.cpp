#include "framework.h"
#include "CheckBox.h"

#include "common.h"

#define CHECKBOX_ANIMATION_DURATION 125

void CheckBox::drawItem(HDC hDC, UINT itemState, RECT& rcItem)
{
	CHECKBOXSTATES state = CBS_UNCHECKEDNORMAL;
	if (itemState & ODS_SELECTED)
	{
		state = CBS_UNCHECKEDPRESSED;
	}
	else if (itemState & ODS_HOTLIGHT || isTracking)
	{
		state = CBS_UNCHECKEDHOT;
	}
	if (check)
	{
		state = (CHECKBOXSTATES)(state + CBS_CHECKEDNORMAL - CBS_UNCHECKEDNORMAL);
	}
	BP_ANIMATIONPARAMS animParams = { sizeof(BP_ANIMATIONPARAMS),0, BPAS_LINEAR, CHECKBOX_ANIMATION_DURATION };
	HDC hdcFrom, hdcTo;
	HANIMATIONBUFFER hbpAnimation = BeginBufferedAnimation(hButton, hDC, &rcItem, BPBF_COMPATIBLEBITMAP, NULL, &animParams, &hdcFrom, &hdcTo);
	if (lastState == state || hbpAnimation == nullptr)
	{
		int width = rcItem.right - rcItem.left;
		int height = rcItem.bottom - rcItem.top;
		HBITMAP bmp = CreateCompatibleBitmap(hDC, width, height);
		HDC hDCMem = CreateCompatibleDC(hDC);
		SelectObject(hDCMem, bmp);
		SelectObject(hDCMem, hFont);
		drawCheckBox(hDCMem, state, rcItem);
		BitBlt(hDC, rcItem.left, rcItem.top, width, height, hDCMem, 0, 0, SRCCOPY);
		DeleteDC(hDCMem);
		DeleteObject(bmp);
	}
	else
	{
		SelectObject(hdcFrom, hFont);
		SelectObject(hdcTo, hFont);
		drawCheckBox(hdcFrom, lastState, rcItem);
		drawCheckBox(hdcTo, state, rcItem);
		BufferedPaintStopAllAnimations(hButton);
		EndBufferedAnimation(hbpAnimation, TRUE);
	}
	lastState = state;
}

void CheckBox::setCheck(bool check)
{
	this->check = check;
	InvalidateRect(hButton, nullptr, FALSE);
}

bool CheckBox::getCheck()
{
	return check;
}

void CheckBox::drawCheckBox(HDC hDC, CHECKBOXSTATES state, RECT& rcItem)
{
	int height = rcItem.bottom - rcItem.top;

	FillRect(hDC, &rcItem, hBkgBrush);

	RECT rcCkeckBox = rcItem;
	rcCkeckBox.right = rcCkeckBox.left + height;
	DrawThemeBackground(hButtonTheme, hDC, BP_CHECKBOX, state, &rcCkeckBox, 0);

	SetBkMode(hDC, TRANSPARENT);
	RECT textRect = rcItem;
	textRect.left += height + 1;
	TCHAR str[20];
	GetWindowText(hButton, str, 20);
	DrawText(hDC, str, wcslen(str), &textRect, DT_SINGLELINE | DT_VCENTER | DT_LEFT);
}