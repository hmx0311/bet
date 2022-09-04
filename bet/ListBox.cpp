#include "framework.h"
#include "ListBox.h"

#include "common.h"

#include <windowsx.h>

WNDPROC defListBoxProc;

LRESULT CALLBACK listBoxProc(HWND hListBox, UINT message, WPARAM wParam, LPARAM lParam)
{
	ListBox* listBox = (ListBox*)GetWindowLongPtr(hListBox, GWLP_USERDATA);
	if (message == WM_ERASEBKGND)
	{
		RECT rect;
		GetClientRect(hListBox, &rect);
		rect.top += (SendMessage(hListBox, LB_GETCOUNT, 0, 0) - GetScrollPos(hListBox, SB_VERT)) * listItemHeight;
		if (rect.top < rect.bottom)
		{
			FillRect((HDC)wParam, &rect, GetSysColorBrush(COLOR_WINDOW));
		}
		return (LRESULT)TRUE;
	}
	if (listBox == nullptr)
	{
		return CallWindowProc(defListBoxProc, hListBox, message, wParam, lParam);
	}
	return listBox->wndProc(message, wParam, lParam);
}

LRESULT ListBox::wndProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	return CallWindowProc(defListBoxProc, hListBox, message, wParam, lParam);
}

void ListBox::drawItem(HDC hDC, int itemID, UINT itemState, ULONG_PTR itemData, RECT& rcItem)
{
	RECT rcContent = { 0,0, rcItem.right, rcItem.bottom - rcItem.top };

	FillRect(hDC, &rcContent, GetSysColorBrush(itemState & ODS_SELECTED ? COLOR_HIGHLIGHT : COLOR_WINDOW));

	SetBkMode(hDC, TRANSPARENT);
	COLORREF color = itemData >> 8;
	SetTextColor(hDC, itemState & ODS_SELECTED ? ::GetSysColor(COLOR_HIGHLIGHTTEXT) : color);
	TCHAR sText[30];
	getText(itemID, sText);
	BYTE style = itemData & 0xff;
	DrawText(hDC, sText, -1, &rcContent, style | DT_SINGLELINE);
}

void ListBox::attach(HWND hListBox)
{
	this->hListBox = hListBox;
	SetWindowLongPtr(hListBox, GWLP_USERDATA, (LONG_PTR)this);
	SetWindowLongPtr(hListBox, GWLP_WNDPROC, (LONG_PTR)listBoxProc);
}

HWND ListBox::getHwnd()
{
	return hListBox;
}

int ListBox::addString(LPCTSTR lpszItem, BYTE style, COLORREF color)
{
	int index = SendMessage(hListBox, LB_ADDSTRING, 0, (LPARAM)lpszItem);
	SendMessage(hListBox, LB_SETITEMDATA, index, ((color << 8) | style));
	return index;
}

void ListBox::getText(int nIndex, LPWSTR str)
{
	SendMessage(hListBox, LB_GETTEXT, nIndex, (LPARAM)str);
}

void ListBox::setCurSel(int nSelect)
{
	SendMessage(hListBox, LB_SETCURSEL, nSelect, 0);
}

int ListBox::getCurSel()
{
	return (int)SendMessage(hListBox, LB_GETCURSEL, 0, 0);
}