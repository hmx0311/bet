#include "framework.h"
#include "listBox.h"

#include "common.h"
#include "bet.h"

#include <windowsx.h>

using namespace std;

#define X_CHANGE (int)(2-53.0f*xScale)

static LRESULT CALLBACK listBoxSubclassProc(HWND hLB, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
	ListBox* listBox = (ListBox*)GetWindowLongPtr(hLB, GWLP_USERDATA);
	if (msg == WM_ERASEBKGND)
	{
		RECT rect;
		GetClientRect(hLB, &rect);
		rect.top += (ListBox_GetCount(hLB) - GetScrollPos(hLB, SB_VERT)) * listItemHeight;
		if (rect.top < rect.bottom)
		{
			FillRect((HDC)wParam, &rect, GetSysColorBrush(COLOR_WINDOW));
		}
		return (LRESULT)TRUE;
	}
	return listBox->wndProc(msg, wParam, lParam);
}


//class ListBox

void ListBox::attach(HWND hLB)
{
	this->hLB = hLB;
	SetWindowLongPtr(hLB, GWLP_USERDATA, (LONG_PTR)this);
	SetWindowSubclass(hLB, listBoxSubclassProc, 0, 0);
	RECT rect;
	GetWindowRect(hLB, &rect);
	maxDisplayedItemCnt = (rect.bottom - rect.top) / listItemHeight;
}

LRESULT ListBox::wndProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_DPICHANGED_AFTERPARENT:
		{
			ListBox_SetItemHeight(hLB, 0, listItemHeight);
			RECT rect;
			GetWindowRect(hLB, &rect);
			MapWindowRect(HWND_DESKTOP, hLB, &rect);
			SetWindowPos(hLB, nullptr, 0, 0, rect.right - rect.left, listItemHeight * maxDisplayedItemCnt - 2 * rect.top, SWP_NOMOVE | SWP_NOZORDER | SWP_NOREDRAW);
			break;
		}
	}
	return DefSubclassProc(hLB, msg, wParam, lParam);
}

void ListBox::drawItem(HDC hDC, int itemID, UINT itemState, ULONG_PTR itemData, RECT& rcItem)
{
	FillRect(hDC, &rcItem, GetSysColorBrush(itemState & ODS_SELECTED ? COLOR_HIGHLIGHT : COLOR_WINDOW));

	SetBkMode(hDC, TRANSPARENT);
	COLORREF color = itemData >> 8;
	if (color == 0)
	{
		color = GetSysColor(COLOR_WINDOWTEXT);
	}
	SetTextColor(hDC, itemState & ODS_SELECTED ? ::GetSysColor(COLOR_HIGHLIGHTTEXT) : color);
	TCHAR sText[30];
	ListBox_GetText(hLB, itemID, sText);
	BYTE style = itemData & 0xff;
	DrawText(hDC, sText, -1, &rcItem, style | DT_SINGLELINE);
}

HWND ListBox::getHwnd()
{
	return hLB;
}

int ListBox::addString(PCTSTR pszItem, BYTE style, COLORREF color)
{
	int index = ListBox_AddString(hLB, pszItem);
	ListBox_SetItemData(hLB, index, ((color << 8) | style));
	return index;
}

void ListBox::setCurSel(int nSelect)
{
	ListBox_SetCurSel(hLB, nSelect);
}


//class BetList

void BetList::attach(HWND hLB, HWND hAllBoughtButton, NumericEdit* boughtEdit)
{
	ListBox::attach(hLB);
	MakeDragList(hLB);
	this->hAllBoughtButton = hAllBoughtButton;
	this->boughtEdit = boughtEdit;
	GetWindowRect(hLB, &rcListBox);
	MapWindowRect(HWND_DESKTOP, GetParent(hLB), &rcListBox);
	resetContent();
}

LRESULT BetList::wndProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_DPICHANGED_AFTERPARENT:
		{
			ListBox_SetItemHeight(hLB, 0, listItemHeight);
			GetWindowRect(hLB, &rcListBox);
			MapWindowRect(HWND_DESKTOP, hLB, &rcListBox);
			SetWindowPos(hLB, nullptr, 0, 0, rcListBox.right - rcListBox.left, listItemHeight * maxDisplayedItemCnt - 2 * rcListBox.top, SWP_NOMOVE | SWP_NOZORDER | SWP_NOREDRAW);
			MapWindowRect(hLB, GetParent(hLB), &rcListBox);
			break;
		}
	case WM_KEYDOWN:
		switch (LOWORD(wParam))
		{
		case VK_UP:
		case VK_LEFT:
			{
				int curSel = getCurSel();
				if (isEmpty() || curSel == 2 || curSel == 5 && betsSize == 0)
				{
					return (LRESULT)TRUE;
				}
				if (curSel == 5 + betsSize)
				{
					setCurSel(1 + betsSize);
					return (LRESULT)TRUE;
				}
				break;
			}
		case VK_DOWN:
		case VK_RIGHT:
			if (isEmpty())
			{
				return (LRESULT)TRUE;
			}
			if (getCurSel() == 1 + betsSize)
			{
				if (bankersSize == 0)
				{
					return (LRESULT)TRUE;
				}
				setCurSel(5 + betsSize);
				return (LRESULT)TRUE;
			}
			break;
		case VK_DELETE:
			if (!(GetAsyncKeyState(VK_LBUTTON) & 0x8000) && getCurSel() > 0)
			{
				SendMessage(GetParent(hLB), WM_COMMAND, ID_DELETE, 0);
			}
			return (LRESULT)TRUE;
		}
		break;
	case WM_LBUTTONDBLCLK:
		{
			int curSel = getCurSel();
			if (curSel < betsSize + 5)
			{
				return (LRESULT)TRUE;
			}
			RECT rect;
			ListBox_GetItemRect(hLB, curSel, &rect);
			if (!PtInRect(&rect, { GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam) }))
			{
				return (LRESULT)TRUE;
			}
			ShowWindow(hAllBoughtButton, SW_HIDE);
			ShowWindow(boughtEdit->getHwnd(), SW_SHOW);
			TCHAR str[20];
			ListBox_GetText(hLB, curSel, str);
			int i;
			for (i = 12; str[i] == ' '; i++);
			boughtEdit->setText(&str[i]);
			boughtEdit->setSel(0, -1);
			SetFocus(boughtEdit->getHwnd());
			return (LRESULT)TRUE;
		}
	case WM_CONTEXTMENU:
		{
			int curSel = getCurSel();
			if (curSel < 0)
			{
				return (LRESULT)TRUE;
			}
			RECT rect;
			ListBox_GetItemRect(hLB, curSel, &rect);
			MapWindowRect(hLB, HWND_DESKTOP, &rect);
			POINT pos = { lParam == -1 ? (rect.left + rect.right) / 2 : GET_X_LPARAM(lParam),lParam == -1 ? (rect.top + rect.bottom) / 2 : GET_Y_LPARAM(lParam) };
			if (!PtInRect(&rect, pos))
			{
				return (LRESULT)TRUE;
			}
			HMENU menu = CreatePopupMenu();
			AppendMenu(menu, 0, ID_DELETE, _T("删除(&D)"));
			TrackPopupMenu(menu, 0, pos.x, pos.y, 0, GetParent(hLB), nullptr);
			DestroyMenu(menu);
			return (LRESULT)TRUE;
		}
	case WM_KILLFOCUS:
		if ((HWND)wParam != hAllBoughtButton && (HWND)wParam != boughtEdit->getHwnd())
		{
			if (isDragging)
			{
				cancelDrag();
				SetCursor(nullptr);
			}
			setCurSel(-1);
			ShowWindow(hAllBoughtButton, SW_HIDE);
			RedrawWindow(hLB, nullptr, nullptr, RDW_FRAME | RDW_INVALIDATE);
		}
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case ID_CONFIRM:
			if (!isDragging)
			{
				int curSel = getCurSel();
				int curScroll = GetScrollPos(hLB, SB_VERT);
				if (curSel < betsSize + 5 || curSel < curScroll || curSel >= curScroll + maxDisplayedItemCnt)
				{
					return (LRESULT)TRUE;
				}
				ShowWindow(hAllBoughtButton, SW_HIDE);
				ShowWindow(boughtEdit->getHwnd(), SW_SHOW);
				TCHAR str[20];
				ListBox_GetText(hLB, curSel, str);
				int i;
				for (i = 12; str[i] == ' '; i++);
				boughtEdit->setText(&str[i]);
				boughtEdit->setSel(0, -1);
				SetFocus(boughtEdit->getHwnd());
				return (LRESULT)TRUE;
			}
			break;
		}
		break;
	}

	LRESULT result = DefSubclassProc(hLB, msg, wParam, lParam);

	switch (msg)
	{
	case WM_PAINT:
	case WM_NCPAINT:
		if (GetFocus() == hLB || getCurSel() != -1)
		{
			HPEN hPen = CreatePen(PS_SOLID, 2, GetSysColor(COLOR_HIGHLIGHT));
			HDC hDC = GetDCEx(hLB, nullptr, DCX_WINDOW | DCX_PARENTCLIP);
			SelectObject(hDC, hPen);
			SelectObject(hDC, GetStockBrush(NULL_BRUSH));
			Rectangle(hDC, 1, 1, rcListBox.right - rcListBox.left, rcListBox.bottom - rcListBox.top);
			DeleteObject(hPen);
			ReleaseDC(hLB, hDC);
		}
		break;
	case WM_KEYDOWN:
		switch (LOWORD(wParam))
		{
		case VK_UP:
		case VK_LEFT:
		case VK_DOWN:
		case VK_RIGHT:
			if (getCurSel() == -1)
			{
				if (betsSize == 0)
				{
					setCurSel(5);
					break;
				}
				int curSel = ListBox_GetCurSel(hLB);
				if (curSel < 2)
				{
					setCurSel(2);
					break;
				}
				if (bankersSize == 0)
				{
					setCurSel(betsSize + 1);
					break;
				}
				setCurSel(betsSize + (curSel > betsSize + 2 ? 5 : 1));
			}
			break;
		}
		break;
	case WM_VSCROLL:
	case WM_MOUSEWHEEL:
		if (getCurSel() != -1)
		{
			int selLineIdx = getCurSel() - GetScrollPos(hLB, SB_VERT);
			if (selLineIdx < 0 || selLineIdx >= maxDisplayedItemCnt)
			{
				if (GetFocus() == boughtEdit->getHwnd())
				{
					ShowWindow(boughtEdit->getHwnd(), SW_HIDE);
					SetFocus(hLB);
				}
			}
		}
		break;
	}
	return result;
}

void BetList::drawItem(HDC hDC, int itemID, UINT itemState, ULONG_PTR itemData, RECT& rcItem)
{
	if ((itemID < 2 || betsSize + 1 < itemID && itemID < betsSize + 5) && itemState & ODS_SELECTED)
	{
		itemState -= ODS_SELECTED;
		ShowWindow(hAllBoughtButton, SW_HIDE);
	}
	if (itemState & ODS_SELECTED)
	{
		if (rcItem.top<0 || rcItem.bottom>rcListBox.bottom - rcListBox.top)
		{
			if (GetFocus() != boughtEdit->getHwnd())
			{
				ShowWindow(hAllBoughtButton, SW_HIDE);
			}
			return;
		}
		int posY = rcListBox.top + rcItem.top + 2;
		if (IsWindowVisible(boughtEdit->getHwnd()))
		{
			SetWindowPos(boughtEdit->getHwnd(), HWND_TOP, rcListBox.left + rcItem.right - listItemHeight + X_CHANGE, posY, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
		}
		else
		{
			int posX = rcListBox.left + rcItem.right - listItemHeight + 2;
			if (itemID > betsSize + 2)
			{
				SetWindowPos(hAllBoughtButton, HWND_TOP, posX, posY, listItemHeight, listItemHeight, SWP_SHOWWINDOW);
			}
			else
			{
				ShowWindow(hAllBoughtButton, SW_HIDE);
			}
		}
		FillRect(hDC, &rcItem, GetSysColorBrush(COLOR_HIGHLIGHT));
	}
	else
	{
		FillRect(hDC, &rcItem, GetSysColorBrush(COLOR_WINDOW));
	}
	if (itemID == betsSize + 2)
	{
		rcItem.top += (rcItem.bottom - rcItem.top) / 2;
		rcItem.bottom = rcItem.top + 1;
		FillRect(hDC, &rcItem, GetSysColorBrush(COLOR_WINDOWTEXT));
		return;
	}
	if (itemID > betsSize + 4)
	{
		rcItem.right -= listItemHeight + xScale;
	}
	SetBkMode(hDC, TRANSPARENT);
	COLORREF color = itemData >> 8;
	if (color == 0)
	{
		color = GetSysColor(COLOR_WINDOWTEXT);
	}
	SetTextColor(hDC, itemState & ODS_SELECTED ? ::GetSysColor(COLOR_HIGHLIGHTTEXT) : color);
	if (itemID == 0 || itemID == betsSize + 3)
	{
		SelectObject(hDC, hBoldFont);
	}
	TCHAR sText[30];
	ListBox_GetText(hLB, itemID, sText);
	BYTE style = itemData & 0xff;
	DrawText(hDC, sText, -1, &rcItem, style);
}

BOOL BetList::beginDrag(POINT ptCursor)
{
	int dragIdx = LBItemFromPt(hLB, ptCursor, FALSE);
	return dragIdx > 1 && dragIdx < 2 + betsSize && betsSize>1 || dragIdx > betsSize + 4 && bankersSize > 1;
}

UINT BetList::dragging(POINT ptCursor)
{
	int curSel = getCurSel();
	if (curSel == -1)
	{
		return DL_CURSORSET;
	}
	ScreenToClient(hLB, &ptCursor);
	RECT rc;
	ListBox_GetItemRect(hLB, 0, &rc);
	int dragIdx;
	if (ptCursor.x<0 || ptCursor.x>rc.right)
	{
		dragIdx = -1;
	}
	else
	{
		dragIdx = (ptCursor.y + listItemHeight / 2) / listItemHeight;
		int curScroll = GetScrollPos(hLB, SB_VERT);
		if (curSel < betsSize + 2)
		{
			isDragging = true;
			if (dragIdx <= 0)
			{
				if (curScroll > 1)
				{
					curScroll--;
					ListBox_SetTopIndex(hLB, curScroll);
				}
				dragIdx = 1;
			}
			else if (dragIdx >= maxDisplayedItemCnt)
			{
				if (curScroll + maxDisplayedItemCnt < betsSize + 3)
				{
					curScroll++;
					ListBox_SetTopIndex(hLB, curScroll);
				}
				dragIdx = maxDisplayedItemCnt - 1;
			}
			dragIdx += curScroll;
			if (dragIdx < 2)
			{
				dragIdx = 2;
			}
			else if (dragIdx > betsSize + 2)
			{
				dragIdx = -1;
			}
		}
		else
		{
			if (!isDragging)
			{
				addString(_T(""));
				isDragging = true;
			}
			if (dragIdx <= 0)
			{
				if (curScroll > betsSize + 4)
				{
					curScroll--;
					ListBox_SetTopIndex(hLB, curScroll);
				}
				dragIdx = 1;
			}
			else if (dragIdx >= maxDisplayedItemCnt)
			{
				ListBox_SetTopIndex(hLB, curScroll + 1);
				int curScroll = GetScrollPos(hLB, SB_VERT);
				dragIdx = maxDisplayedItemCnt - 1;
			}
			dragIdx += curScroll;
			if (dragIdx < betsSize + 5)
			{
				dragIdx = -1;
			}
			else if (dragIdx > betsSize + bankersSize + 5)
			{
				dragIdx = betsSize + bankersSize + 5;
			}
		}
	}
	if (dragIdx < 0 || dragIdx == curSel || dragIdx == curSel + 1)
	{
		if (lastDragIdx != -1)
		{
			HDC hDC = GetDC(hLB);
			eraseDragLine(hDC);
			ReleaseDC(hLB, hDC);
			lastDragIdx = -1;
		}
		return DL_STOPCURSOR;
	}
	if (dragIdx != lastDragIdx)
	{
		HDC hDC = GetDC(hLB);
		if (lastDragIdx != -1)
		{
			eraseDragLine(hDC);
		}
		RECT rcLine;
		ListBox_GetItemRect(hLB, dragIdx, &rcLine);
		rcLine.top--;
		rcLine.bottom = rcLine.top + 2;
		DrawFocusRect(hDC, &rcLine);
		ReleaseDC(hLB, hDC);
		lastDragIdx = dragIdx;
	}
	return DL_MOVECURSOR;
}

int BetList::dropped(POINT ptCursor)
{
	if (!isDragging)
	{
		return -1;
	}
	isDragging = false;
	int curSel = getCurSel();
	if (lastDragIdx != -1)
	{
		HDC hDC = GetDC(hLB);
		eraseDragLine(hDC);
		ReleaseDC(hLB, hDC);
		TCHAR str[20];
		ListBox_GetText(hLB, curSel, str);
		LRESULT itemData = ListBox_GetItemData(hLB, curSel);
		ListBox_DeleteString(hLB, curSel);
		if (curSel < lastDragIdx)
		{
			lastDragIdx--;
		}
		ListBox_InsertString(hLB, lastDragIdx, str);
		ListBox_SetItemData(hLB, lastDragIdx, itemData);
		setCurSel(lastDragIdx);
	}
	if (curSel > betsSize + 2)
	{
		ListBox_DeleteString(hLB, betsSize + bankersSize + 5);
		if (GetScrollPos(hLB, SB_VERT) >= betsSize + bankersSize + 5 - maxDisplayedItemCnt)
		{
			ListBox_SetTopIndex(hLB, betsSize + bankersSize + 5 - maxDisplayedItemCnt);
		}
	}
	int temp = lastDragIdx;
	lastDragIdx = -1;
	return temp;
}

void BetList::cancelDrag()
{
	if (!isDragging)
	{
		return;
	}
	isDragging = false;
	int curSel = getCurSel();
	if (lastDragIdx != -1)
	{
		HDC hDC = GetDC(hLB);
		eraseDragLine(hDC);
		ReleaseDC(hLB, hDC);
		lastDragIdx = -1;
	}
	if (getCurSel() > betsSize + 2)
	{
		ListBox_DeleteString(hLB, betsSize + bankersSize + 5);
		if (GetScrollPos(hLB, SB_VERT) >= betsSize + bankersSize + 5 - maxDisplayedItemCnt)
		{
			ListBox_SetTopIndex(hLB, betsSize + bankersSize + 5 - maxDisplayedItemCnt);
		}
	}
}

int BetList::getBetsSize()
{
	return betsSize;
}

int BetList::getBankersSize()
{
	return bankersSize;
}

int BetList::getCurSel()
{
	int curSel = ListBox_GetCurSel(hLB);
	if (curSel < 2 || betsSize + 1 < curSel && curSel < betsSize + 5)
	{
		return -1;
	}
	return curSel;
}

void BetList::addBet(PCTSTR str)
{
	insertString(betsSize + 2, str, DT_CENTER);
	betsSize++;
	ListBox_SetTopIndex(hLB, betsSize > maxDisplayedItemCnt - 2 ? betsSize + 2 - maxDisplayedItemCnt : 0);
}

void BetList::addBanker(PCTSTR str)
{
	int nIndex = addString(str, DT_RIGHT, RGB(255, 0, 0));
	bankersSize++;
	ListBox_SetTopIndex(hLB, betsSize + bankersSize + 6 - maxDisplayedItemCnt);
}

void BetList::updateBanker(int nIndex, PCTSTR pszItem, COLORREF color)
{
	insertString(nIndex, pszItem, DT_RIGHT, color);
	ListBox_DeleteString(hLB, nIndex + 1);
	ShowWindow(hAllBoughtButton, SW_HIDE);
}

pair<bool, int> BetList::deleteSel()
{
	int lineIdx = getCurSel();
	pair<bool, int> result;
	result.first = lineIdx < betsSize + 2;
	if (result.first)
	{
		result.second = lineIdx - 2;
		betsSize--;
	}
	else
	{
		result.second = lineIdx - betsSize - 5;
		bankersSize--;
	}
	ListBox_DeleteString(hLB, lineIdx);
	if (GetScrollPos(hLB, SB_VERT) >= betsSize + bankersSize + 5 - maxDisplayedItemCnt)
	{
		ListBox_SetTopIndex(hLB, betsSize + bankersSize + 5 - maxDisplayedItemCnt);
	}
	ShowWindow(hAllBoughtButton, SW_HIDE);
	ShowWindow(boughtEdit->getHwnd(), SW_HIDE);
	return result;
}

bool BetList::isEmpty()
{
	return betsSize + bankersSize == 0;
}

void BetList::resetContent()
{
	ListBox_ResetContent(hLB);
	betsSize = 0;
	bankersSize = 0;
	addString(_T("下注"), DT_CENTER);
	addString(_T("赔率  已投入 "), DT_CENTER);
	addString(_T(""));
	addString(_T("庄家"), DT_CENTER);
	addString(_T("赔率 已投入   已买"));
}

void BetList::eraseDragLine(HDC hDC)
{
	RECT rcLastLine;
	ListBox_GetItemRect(hLB, lastDragIdx, &rcLastLine);
	rcLastLine.top--;
	rcLastLine.bottom = rcLastLine.top + 2;
	FillRect(hDC, &rcLastLine, GetSysColorBrush(COLOR_WINDOW));
}

int BetList::insertString(int nIndex, PCTSTR pszItem, BYTE style, COLORREF color)
{
	int index = ListBox_InsertString(hLB, nIndex, pszItem);
	ListBox_SetItemData(hLB, index, ((color << 8) | style));
	return index;
}