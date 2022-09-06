#include "framework.h"
#include "BetList.h"

#include "common.h"

#include <windowsx.h>
#include <CommCtrl.h>

using namespace std;

int X_MOVE;
#define Y_MOVE (int)(-5.5f*yScale)
#define X_ALL_BOUGHT (int)roundf(157.7f*xScale)
#define Y_ALL_BOUGHT (int)roundf(-3.2f*yScale)
#define X_CHANGE (int)(2-54.0f*xScale)

int BetList::maxDisplayedItemCnt;

void BetList::attach(HWND hListBox, HWND hMoveSpin, HWND hAllBoughtButton, NumericEdit* boughtEdit)
{
	ListBox::attach(hListBox);
	this->hMoveSpin = hMoveSpin;
	this->hAllBoughtButton = hAllBoughtButton;
	this->boughtEdit = boughtEdit;
	GetWindowRect(hListBox, &rcListBox);
	MapWindowRect(HWND_DESKTOP, GetParent(hListBox), &rcListBox);
	resetContent();
}

LRESULT BetList::wndProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
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
			ShowWindow(hMoveSpin, SW_HIDE);
			ShowWindow(hAllBoughtButton, SW_HIDE);
			ShowWindow(boughtEdit->getHwnd(), SW_SHOW);
			TCHAR str[20];
			getText(curSel, str);
			int i;
			for (i = 12; str[i] == ' '; i++);
			boughtEdit->setText(&str[i]);
			boughtEdit->setSel(0, -1);
			SetFocus(boughtEdit->getHwnd());
			return (LRESULT)TRUE;
		}
	case WM_RBUTTONDOWN:
		{
			int curSel = getCurSel();
			if (wParam & MK_LBUTTON || getCurSel() < 0)
			{
				return (LRESULT)TRUE;
			}
			RECT rect;
			getItemRect(curSel, &rect);
			POINT pos = { GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam) };
			if (!PtInRect(&rect, pos))
			{
				return (LRESULT)TRUE;
			}
			ClientToScreen(hListBox, &pos);
			HMENU menu = CreatePopupMenu();
			AppendMenu(menu, 0, ID_DELETE, _T("删除"));
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
			int oldScroll = getScrollPos();
			setTopIndex(((short)HIWORD(wParam)) < 0 ? oldScroll + 1 : oldScroll - 1);
			if (oldScroll != getScrollPos() && getCurSel() != -1)
			{
				int selLineIdx = getCurSel() - getScrollPos();
				if (selLineIdx < 0 || selLineIdx >= maxDisplayedItemCnt)
				{
					setCurSel(-1);
					if (GetFocus() == boughtEdit->getHwnd())
					{
						ShowWindow(boughtEdit->getHwnd(), SW_HIDE);
						SetFocus(hListBox);
					}
					else
					{
						ShowWindow(hMoveSpin, SW_HIDE);
						ShowWindow(hAllBoughtButton, SW_HIDE);
					}
				}
			}
			return (LRESULT)TRUE;
		}
	case WM_KILLFOCUS:
		{
			HWND hFocus = GetFocus();
			if (hFocus != hAllBoughtButton && hFocus != boughtEdit->getHwnd())
			{
				setCurSel(-1);
				ShowWindow(hMoveSpin, SW_HIDE);
				ShowWindow(hAllBoughtButton, SW_HIDE);
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
				ShowWindow(hMoveSpin, SW_HIDE);
				ShowWindow(hAllBoughtButton, SW_HIDE);
				ShowWindow(boughtEdit->getHwnd(), SW_SHOW);
				TCHAR str[20];
				getText(curSel, str);
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
				int curSel = ListBox::getCurSel();
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
				setCurSel(getScrollPos());
			}
			else if (y > rcListBox.bottom - rcListBox.top)
			{
				setCurSel(getScrollPos() + maxDisplayedItemCnt - 1);
			}
		}
		break;
	case WM_VSCROLL:
		if (getCurSel() != -1)
		{
			int selLineIdx = getCurSel() - getScrollPos();
			if (selLineIdx < 0 || selLineIdx >= maxDisplayedItemCnt)
			{
				setCurSel(-1);
				if (GetFocus() == boughtEdit->getHwnd())
				{
					ShowWindow(boughtEdit->getHwnd(), SW_HIDE);
					SetFocus(hListBox);
				}
				else
				{
					ShowWindow(hMoveSpin, SW_HIDE);
					ShowWindow(hAllBoughtButton, SW_HIDE);
				}
			}
		}
	}
	return result;
}

void BetList::drawItem(HDC hDC, int itemID, UINT itemState, ULONG_PTR itemData, RECT& rcItem)
{
	RECT rcContent = { 0,0, rcItem.right, rcItem.bottom - rcItem.top };
	if ((itemID < 2 || betsSize + 1 < itemID && itemID < betsSize + 5) && itemState & ODS_SELECTED)
	{
		itemState -= ODS_SELECTED;
		ShowWindow(hMoveSpin, SW_HIDE);
		ShowWindow(hAllBoughtButton, SW_HIDE);
	}
	if (itemState & ODS_SELECTED)
	{
		int posY = rcListBox.top + rcItem.top + 2;
		if (IsWindowVisible(boughtEdit->getHwnd()))
		{
			SetWindowPos(boughtEdit->getHwnd(), nullptr, rcListBox.left + rcItem.right + X_CHANGE, posY, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
		}
		else
		{
			SetWindowPos(hMoveSpin, nullptr, rcListBox.left + X_MOVE, posY + Y_MOVE, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);
			if (itemID > betsSize + 2)
			{
				SetWindowPos(hAllBoughtButton, nullptr, rcListBox.left + X_ALL_BOUGHT, posY + Y_ALL_BOUGHT, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);
			}
			else
			{
				ShowWindow(hAllBoughtButton, SW_HIDE);
			}
		}
		FillRect(hDC, &rcContent, GetSysColorBrush(COLOR_HIGHLIGHT));
	}
	else
	{
		FillRect(hDC, &rcContent, GetSysColorBrush(COLOR_WINDOW));
	}
	if (itemID == betsSize + 2)
	{
		MoveToEx(hDC, 0, rcContent.bottom / 2, nullptr);
		LineTo(hDC, rcContent.right, rcContent.bottom / 2);
		return;
	}
	SetBkMode(hDC, TRANSPARENT);
	COLORREF color = itemData >> 8;
	SetTextColor(hDC, itemState & ODS_SELECTED ? ::GetSysColor(COLOR_HIGHLIGHTTEXT) : color);
	TCHAR sText[30];
	getText(itemID, sText);
	BYTE style = itemData & 0xff;
	DrawText(hDC, sText, -1, &rcContent, style);
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
	int curSel = ListBox::getCurSel();
	if (curSel < 2 || betsSize + 1 < curSel && curSel < betsSize + 5)
	{
		return -1;
	}
	return curSel;
}

int BetList::getItemRect(int nIndex, LPRECT lpRect)
{
	return (int)SendMessage(hListBox, LB_GETITEMRECT, nIndex, (LPARAM)lpRect);
}

int BetList::getScrollPos()
{
	return GetScrollPos(hListBox, SB_VERT);
}

void BetList::setTopIndex(int nIndex)
{
	SendMessage(hListBox, LB_SETTOPINDEX, nIndex, 0);
}

void BetList::addBet(LPCTSTR str)
{
	insertString(betsSize + 2, str, DT_CENTER);
	betsSize++;
	setTopIndex(betsSize > maxDisplayedItemCnt - 2 ? betsSize + 2 - maxDisplayedItemCnt : 0);
}

void BetList::addBanker(LPCTSTR str)
{
	int nIndex = addString(str, DT_CENTER, RGB(255, 0, 0));
	bankersSize++;
	setTopIndex(betsSize + bankersSize + 6 - maxDisplayedItemCnt);
}

void BetList::updateBanker(int nIndex, LPCTSTR lpszItem, COLORREF color)
{
	insertString(nIndex, lpszItem, DT_CENTER, color);
	SendMessage(hListBox, LB_DELETESTRING, nIndex + 1, 0);
	ShowWindow(hMoveSpin, SW_HIDE);
	ShowWindow(hAllBoughtButton, SW_HIDE);
}

int BetList::insertString(int nIndex, LPCTSTR lpszItem, BYTE style, COLORREF color)
{
	int index = SendMessage(hListBox, LB_INSERTSTRING, nIndex, (LPARAM)lpszItem);
	SendMessage(hListBox, LB_SETITEMDATA, index, ((color << 8) | style));
	return index;
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
		getText(swapIdx + 1, str);
		insertString(swapIdx, str, -1);
		SendMessage(hListBox, LB_DELETESTRING, swapIdx + 2, 0);
		setCurSel(targetLineIdx);
		return swapIdx - 2;
	}
	int bankerIdx = swapIdx - betsSize - 5;
	if (bankerIdx >= 0 && bankerIdx < bankersSize - 1)
	{
		TCHAR str[20];
		getText(swapIdx + 1, str);
		COLORREF color = SendMessage(hListBox, LB_GETITEMDATA, swapIdx + 1, 0) >> 8;
		insertString(swapIdx, str, -1, color);
		SendMessage(hListBox, LB_DELETESTRING, swapIdx + 2, 0);
		setCurSel(targetLineIdx);
		return bankerIdx;
	}
	return -1;
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
	SendMessage(hListBox, LB_DELETESTRING, lineIdx, 0);
	if (getScrollPos() >= betsSize + bankersSize + 5 - maxDisplayedItemCnt)
	{
		setTopIndex(betsSize + bankersSize + 5 - maxDisplayedItemCnt);
	}
	ShowWindow(hMoveSpin, SW_HIDE);
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
	SendMessage(hListBox, LB_RESETCONTENT, 0, 0);
	betsSize = 0;
	bankersSize = 0;
	addString(_T("下注"), DT_CENTER);
	addString(_T("赔率   数量  "), DT_CENTER);
	addString(_T(""));
	addString(_T("庄家"), DT_CENTER);
	addString(_T("赔率  数量    已买"));
}
