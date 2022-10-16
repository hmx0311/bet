#include "framework.h"
#include "listBox.h"

#include "common.h"
#include "bet.h"

#include <windowsx.h>
#include <CommCtrl.h>

using namespace std;

#define X_CHANGE (int)(2-53.0f*xScale)

static LRESULT CALLBACK listBoxSubclassProc(HWND hListBox, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
	ListBox* listBox = (ListBox*)GetWindowLongPtr(hListBox, GWLP_USERDATA);
	if (msg == WM_ERASEBKGND)
	{
		RECT rect;
		GetClientRect(hListBox, &rect);
		rect.top += (ListBox_GetCount(hListBox) - GetScrollPos(hListBox, SB_VERT)) * listItemHeight;
		if (rect.top < rect.bottom)
		{
			FillRect((HDC)wParam, &rect, GetSysColorBrush(COLOR_WINDOW));
		}
		return (LRESULT)TRUE;
	}
	return listBox->wndProc(msg, wParam, lParam);
}


//class ListBox

void ListBox::attach(HWND hListBox)
{
	this->hListBox = hListBox;
	SetWindowLongPtr(hListBox, GWLP_USERDATA, (LONG_PTR)this);
	SetWindowSubclass(hListBox, listBoxSubclassProc, 0, 0);
	RECT rect;
	GetWindowRect(hListBox, &rect);
	maxDisplayedItemCnt = (rect.bottom - rect.top) / listItemHeight;
}

LRESULT ListBox::wndProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_DPICHANGED_AFTERPARENT:
		{
			ListBox_SetItemHeight(hListBox, 0, listItemHeight);
			RECT rect;
			GetWindowRect(hListBox, &rect);
			MapWindowRect(HWND_DESKTOP, hListBox, &rect);
			SetWindowPos(hListBox, nullptr, 0, 0, rect.right - rect.left, listItemHeight * maxDisplayedItemCnt - 2 * rect.top, SWP_NOMOVE | SWP_NOZORDER | SWP_NOREDRAW);
			break;
		}
	}
	return DefSubclassProc(hListBox, msg, wParam, lParam);
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
	ListBox_GetText(hListBox, itemID, sText);
	BYTE style = itemData & 0xff;
	DrawText(hDC, sText, -1, &rcItem, style | DT_SINGLELINE);
}

HWND ListBox::getHwnd()
{
	return hListBox;
}

int ListBox::addString(PCTSTR pszItem, BYTE style, COLORREF color)
{
	int index = ListBox_AddString(hListBox, pszItem);
	ListBox_SetItemData(hListBox, index, ((color << 8) | style));
	return index;
}

void ListBox::setCurSel(int nSelect)
{
	ListBox_SetCurSel(hListBox, nSelect);
}


//class BetList

void BetList::attach(HWND hListBox, HWND hAllBoughtButton, HWND hMoveUpButton, HWND hMoveDownButton, NumericEdit* boughtEdit)
{
	ListBox::attach(hListBox);
	this->hAllBoughtButton = hAllBoughtButton;
	this->hMoveUpButton = hMoveUpButton;
	this->hMoveDownButton = hMoveDownButton;
	this->boughtEdit = boughtEdit;
	GetWindowRect(hListBox, &rcListBox);
	MapWindowRect(HWND_DESKTOP, GetParent(hListBox), &rcListBox);
	resetContent();
}

LRESULT BetList::wndProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_DPICHANGED_AFTERPARENT:
		{
			ListBox_SetItemHeight(hListBox, 0, listItemHeight);
			GetWindowRect(hListBox, &rcListBox);
			MapWindowRect(HWND_DESKTOP, hListBox, &rcListBox);
			SetWindowPos(hListBox, nullptr, 0, 0, rcListBox.right - rcListBox.left, listItemHeight * maxDisplayedItemCnt - 2 * rcListBox.top, SWP_NOMOVE | SWP_NOZORDER | SWP_NOREDRAW);
			MapWindowRect(hListBox, GetParent(hListBox), &rcListBox);
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
				SendMessage(GetParent(hListBox), WM_COMMAND, ID_DELETE, 0);
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
			ListBox_GetItemRect(hListBox, curSel, &rect);
			if (!PtInRect(&rect, { GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam) }))
			{
				return (LRESULT)TRUE;
			}
			ShowWindow(hAllBoughtButton, SW_HIDE);
			ShowWindow(hMoveUpButton, SW_HIDE);
			ShowWindow(hMoveDownButton, SW_HIDE);
			ShowWindow(boughtEdit->getHwnd(), SW_SHOW);
			TCHAR str[20];
			ListBox_GetText(hListBox, curSel, str);
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
			ListBox_GetItemRect(hListBox, curSel, &rect);
			MapWindowRect(hListBox, HWND_DESKTOP, &rect);
			POINT pos = { lParam == -1 ? (rect.left + rect.right) / 2 : GET_X_LPARAM(lParam),lParam == -1 ? (rect.top + rect.bottom) / 2 : GET_Y_LPARAM(lParam) };
			if (!PtInRect(&rect, pos))
			{
				return (LRESULT)TRUE;
			}
			HMENU menu = CreatePopupMenu();
			AppendMenu(menu, 0, ID_DELETE, _T("删除(&D)"));
			TrackPopupMenu(menu, 0, pos.x, pos.y, 0, GetParent(hListBox), nullptr);
			DestroyMenu(menu);
			return (LRESULT)TRUE;
		}
	case WM_MOUSEMOVE:
		if (wParam == MK_LBUTTON)
		{
			lParam &= MAKEWPARAM(0, 0xffff);
		}
		break;
	case WM_MOUSEWHEEL:
		{
			int oldScroll = GetScrollPos(hListBox, SB_VERT);
			ListBox_SetTopIndex(hListBox, ((short)HIWORD(wParam)) < 0 ? oldScroll + 1 : oldScroll - 1);
			if (oldScroll != GetScrollPos(hListBox, SB_VERT) && getCurSel() != -1)
			{
				int selLineIdx = getCurSel() - GetScrollPos(hListBox, SB_VERT);
				if (selLineIdx < 0 || selLineIdx >= maxDisplayedItemCnt)
				{
					setCurSel(-1);
					if (GetFocus() == boughtEdit->getHwnd())
					{
						ShowWindow(boughtEdit->getHwnd(), SW_HIDE);
						SetFocus(hListBox);
					}
				}
			}
			return (LRESULT)TRUE;
		}
	case WM_KILLFOCUS:
		{
			if ((HWND)wParam != hAllBoughtButton && (HWND)wParam != hMoveUpButton && (HWND)wParam != hMoveDownButton && (HWND)wParam != boughtEdit->getHwnd())
			{
				setCurSel(-1);
				ShowWindow(hAllBoughtButton, SW_HIDE);
				ShowWindow(hMoveUpButton, SW_HIDE);
				ShowWindow(hMoveDownButton, SW_HIDE);
			}
			break;
		}
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case ID_CONFIRM:
			{
				int curSel = getCurSel();
				if (curSel < betsSize + 5)
				{
					return (LRESULT)TRUE;
				}
				ShowWindow(hAllBoughtButton, SW_HIDE);
				ShowWindow(hMoveUpButton, SW_HIDE);
				ShowWindow(hMoveDownButton, SW_HIDE);
				ShowWindow(boughtEdit->getHwnd(), SW_SHOW);
				TCHAR str[20];
				ListBox_GetText(hListBox, curSel, str);
				int i;
				for (i = 12; str[i] == ' '; i++);
				boughtEdit->setText(&str[i]);
				boughtEdit->setSel(0, -1);
				SetFocus(boughtEdit->getHwnd());
				return (LRESULT)TRUE;
			}
		}
		break;
	}

	LRESULT result = DefSubclassProc(hListBox, msg, wParam, lParam);

	switch (msg)
	{
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
				int curSel = ListBox_GetCurSel(hListBox);
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
	case WM_MOUSEMOVE:
		if (wParam == MK_LBUTTON && betsSize + bankersSize + 5 > maxDisplayedItemCnt)
		{
			int y = GET_Y_LPARAM(lParam);
			if (y < 0)
			{
				setCurSel(GetScrollPos(hListBox, SB_VERT));
			}
			else if (y > rcListBox.bottom - rcListBox.top)
			{
				setCurSel(GetScrollPos(hListBox, SB_VERT) + maxDisplayedItemCnt - 1);
			}
		}
		break;
	case WM_VSCROLL:
		if (getCurSel() != -1)
		{
			int selLineIdx = getCurSel() - GetScrollPos(hListBox, SB_VERT);
			if (selLineIdx < 0 || selLineIdx >= maxDisplayedItemCnt)
			{
				setCurSel(-1);
				if (GetFocus() == boughtEdit->getHwnd())
				{
					ShowWindow(boughtEdit->getHwnd(), SW_HIDE);
					SetFocus(hListBox);
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
		ShowWindow(hMoveUpButton, SW_HIDE);
		ShowWindow(hMoveDownButton, SW_HIDE);
	}
	if (itemState & ODS_SELECTED)
	{
		if (rcItem.top<0 || rcItem.bottom>rcListBox.bottom - rcListBox.top)
		{
			setCurSel(-1);
			if (GetFocus() != boughtEdit->getHwnd())
			{
				ShowWindow(hAllBoughtButton, SW_HIDE);
				ShowWindow(hMoveUpButton, SW_HIDE);
				ShowWindow(hMoveDownButton, SW_HIDE);
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
				SetWindowPos(hAllBoughtButton, HWND_TOP, posX, posY, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);
				if (itemID > betsSize + 5 && ListBox_GetTopIndex(hListBox) < itemID)
				{
					SetWindowPos(hMoveUpButton, HWND_TOP, posX, posY - listItemHeight, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);
				}
				else
				{
					ShowWindow(hMoveUpButton, SW_HIDE);
				}
				if (itemID < betsSize + bankersSize + 4 && ListBox_GetTopIndex(hListBox) + maxDisplayedItemCnt - 1 > itemID)
				{
					SetWindowPos(hMoveDownButton, HWND_TOP, posX, posY + listItemHeight, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);
				}
				else
				{
					ShowWindow(hMoveDownButton, SW_HIDE);
				}
			}
			else
			{
				ShowWindow(hAllBoughtButton, SW_HIDE);
				if (itemID > 2 && ListBox_GetTopIndex(hListBox) < itemID)
				{
					if (itemID < betsSize + 1 && ListBox_GetTopIndex(hListBox) + maxDisplayedItemCnt - 1 > itemID)
					{
						SetWindowPos(hMoveUpButton, HWND_TOP, posX, posY - (listItemHeight + 1) / 2, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);
						SetWindowPos(hMoveDownButton, HWND_TOP, posX, posY + listItemHeight / 2, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);
					}
					else
					{
						SetWindowPos(hMoveUpButton, HWND_TOP, posX, posY, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);
						ShowWindow(hMoveDownButton, SW_HIDE);
					}
				}
				else
				{
					if (itemID < betsSize + 1 && ListBox_GetTopIndex(hListBox) + maxDisplayedItemCnt - 1 > itemID)
					{
						ShowWindow(hMoveUpButton, SW_HIDE);
						SetWindowPos(hMoveDownButton, HWND_TOP, posX, posY, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);
					}
					else
					{
						ShowWindow(hMoveUpButton, SW_HIDE);
						ShowWindow(hMoveDownButton, SW_HIDE);
					}
				}
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
	ListBox_GetText(hListBox, itemID, sText);
	BYTE style = itemData & 0xff;
	DrawText(hDC, sText, -1, &rcItem, style);
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
	int curSel = ListBox_GetCurSel(hListBox);
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
	ListBox_SetTopIndex(hListBox, betsSize > maxDisplayedItemCnt - 2 ? betsSize + 2 - maxDisplayedItemCnt : 0);
}

void BetList::addBanker(PCTSTR str)
{
	int nIndex = addString(str, DT_RIGHT, RGB(255, 0, 0));
	bankersSize++;
	ListBox_SetTopIndex(hListBox, betsSize + bankersSize + 6 - maxDisplayedItemCnt);
}

void BetList::updateBanker(int nIndex, PCTSTR pszItem, COLORREF color)
{
	ListBox_DeleteString(hListBox, nIndex);
	insertString(nIndex, pszItem, DT_RIGHT, color);
	ShowWindow(hAllBoughtButton, SW_HIDE);
	ShowWindow(hMoveUpButton, SW_HIDE);
	ShowWindow(hMoveDownButton, SW_HIDE);
}

int BetList::moveSel(bool direction)
{
	int swapIdx = getCurSel();
	int targetLineIdx = direction ? swapIdx - 1 : swapIdx + 1;
	if (direction)
	{
		swapIdx--;
	}
	if (swapIdx > 1 && swapIdx < betsSize + 1)
	{
		TCHAR str[20];
		ListBox_GetText(hListBox, swapIdx + 1, str);
		insertString(swapIdx, str, DT_CENTER);
		ListBox_DeleteString(hListBox, swapIdx + 2);
		setCurSel(targetLineIdx);
		return swapIdx - 2;
	}
	else
	{
		int bankerIdx = swapIdx - betsSize - 5;
		TCHAR str[20];
		ListBox_GetText(hListBox, swapIdx + 1, str);
		COLORREF color = ListBox_GetItemData(hListBox, swapIdx + 1) >> 8;
		insertString(swapIdx, str, DT_RIGHT, color);
		ListBox_DeleteString(hListBox, swapIdx + 2);
		setCurSel(targetLineIdx);
		return bankerIdx;
	}
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
	ListBox_DeleteString(hListBox, lineIdx);
	if (GetScrollPos(hListBox, SB_VERT) >= betsSize + bankersSize + 5 - maxDisplayedItemCnt)
	{
		ListBox_SetTopIndex(hListBox, betsSize + bankersSize + 5 - maxDisplayedItemCnt);
	}
	ShowWindow(hAllBoughtButton, SW_HIDE);
	ShowWindow(hMoveUpButton, SW_HIDE);
	ShowWindow(hMoveDownButton, SW_HIDE);
	ShowWindow(boughtEdit->getHwnd(), SW_HIDE);
	return result;
}

bool BetList::isEmpty()
{
	return betsSize + bankersSize == 0;
}

void BetList::resetContent()
{
	ListBox_ResetContent(hListBox);
	betsSize = 0;
	bankersSize = 0;
	addString(_T("下注"), DT_CENTER);
	addString(_T("赔率  已投入 "), DT_CENTER);
	addString(_T(""));
	addString(_T("庄家"), DT_CENTER);
	addString(_T("赔率 已投入   已买"));
}

int BetList::insertString(int nIndex, PCTSTR pszItem, BYTE style, COLORREF color)
{
	int index = ListBox_InsertString(hListBox, nIndex, pszItem);
	ListBox_SetItemData(hListBox, index, ((color << 8) | style));
	return index;
}