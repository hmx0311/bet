#include "framework.h"
#include "listBox.h"

#include "common.h"
#include "bet.h"

#include <windowsx.h>

using namespace std;

#define LB_SETFOCUS_ANIMATION_DURATION 167
#define LB_KILLFOCUS_ANIMATION_DURATION  250

#define X_CHANGE (int)(-53.0f*xScale)

static LRESULT CALLBACK listBoxSubclassProc(HWND hLB, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
	return ((ListBox*)GetWindowLongPtr(hLB, GWLP_USERDATA))->wndProc(msg, wParam, lParam);
}


//class ListBox

void ListBox::attach(HWND hLB)
{
	this->hLB = hLB;
	SetWindowLongPtr(hLB, GWLP_USERDATA, (LONG_PTR)this);
	SetWindowSubclass(hLB, listBoxSubclassProc, 0, 0);
	GetWindowRect(hLB, &rcLB);
	MapWindowRect(HWND_DESKTOP, GetParent(hLB), &rcLB);
	maxDisplayedItemCnt = (rcLB.bottom - rcLB.top) / listItemHeight;
	ImmAssociateContext(hLB, nullptr);
}

LRESULT ListBox::wndProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_DPICHANGED_AFTERPARENT:
		ListBox_SetItemHeight(hLB, 0, listItemHeight);
		GetWindowRect(hLB, &rcLB);
		MapWindowRect(HWND_DESKTOP, GetParent(hLB), &rcLB);
		rcLB.bottom = rcLB.top + listItemHeight * maxDisplayedItemCnt + 4;
		SetWindowPos(hLB, nullptr, 0, 0, rcLB.right - rcLB.left, rcLB.bottom - rcLB.top, SWP_NOMOVE | SWP_NOZORDER | SWP_NOREDRAW);
		break;
	case WM_ERASEBKGND:
		RECT rect;
		GetClientRect(hLB, &rect);
		rect.top += (ListBox_GetCount(hLB) - GetScrollPos(hLB, SB_VERT)) * listItemHeight;
		if (rect.top < rect.bottom)
		{
			FillRect((HDC)wParam, &rect, GetSysColorBrush(COLOR_WINDOW));
		}
		return LRESULT(TRUE);
	case WM_PAINT:
		if (animationEndTime > clock())
		{
			HDC hDC = GetDCEx(hLB, nullptr, DCX_WINDOW | DCX_PARENTCLIP);
			if (BufferedPaintRenderAnimation(hLB, hDC))
			{
				ReleaseDC(hLB, hDC);
				ValidateRect(hLB, nullptr);
				return 0;
			}
			ReleaseDC(hLB, hDC);
		}
		break;
	}

	LRESULT result = DefSubclassProc(hLB, msg, wParam, lParam);

	switch (msg)
	{
	}
	return result;
}

HWND ListBox::getHwnd() const
{
	return hLB;
}

void ListBox::setCurSel(int nSelect)
{
	ListBox_SetCurSel(hLB, nSelect);
}

void ListBox::drawFocus(bool isFocused)
{
	HDC hDC = GetDCEx(hLB, nullptr, DCX_WINDOW | DCX_PARENTCLIP);
	RECT rcFocus = { rcLB.right - rcLB.left - 2, 0, rcLB.right - rcLB.left, rcLB.bottom - rcLB.top };
	BP_PAINTPARAMS paintParams = { sizeof(BP_PAINTPARAMS), BPPF_NONCLIENT, nullptr, nullptr };
	BP_ANIMATIONPARAMS animParams = { sizeof(BP_ANIMATIONPARAMS), 0, BPAS_LINEAR, isFocused ? LB_SETFOCUS_ANIMATION_DURATION : LB_KILLFOCUS_ANIMATION_DURATION };
	HDC hDCFrom, hDCTo;
	HANIMATIONBUFFER hbpAnimation = BeginBufferedAnimation(hLB, hDC, &rcFocus, BPBF_COMPATIBLEBITMAP, &paintParams, &animParams, &hDCFrom, &hDCTo);
	if (isFocused)
	{
		FillRect(hDCTo, &rcFocus, GetSysColorBrush(COLOR_HIGHLIGHT));
	}
	else
	{
		StretchBlt(hDCTo, rcLB.right - rcLB.left - 2, 0, 2, rcLB.bottom - rcLB.top, hDC, 1, 0, -2, rcLB.bottom - rcLB.top, SRCCOPY);
	}
	if (hDCFrom != nullptr)
	{
		BitBlt(hDCFrom, rcLB.right - rcLB.left - 2, 0, 2, rcLB.bottom - rcLB.top, hDC, rcLB.right - rcLB.left - 2, 0, SRCCOPY);
		animationEndTime = clock() + animParams.dwDuration;
	}
	BufferedPaintStopAllAnimations(hLB);
	EndBufferedAnimation(hbpAnimation, TRUE);
	ReleaseDC(hLB, hDC);
}


//class BetList

BetList::BetList(Button& allBoughtButton, NumericEdit& boughtEdit)
	:allBoughtButton(allBoughtButton), boughtEdit(boughtEdit) {}

void BetList::attach(HWND hLB)
{
	ListBox::attach(hLB);
	MakeDragList(hLB);
	resetContent();
}

LRESULT BetList::wndProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_SETFOCUS:
		if (getCurSel() == -1)
		{
			drawFocus(true);
		}
		break;
	case WM_KILLFOCUS:
		if ((HWND)wParam != allBoughtButton.getHwnd() && (HWND)wParam != boughtEdit.getHwnd())
		{
			if (isDragging)
			{
				cancelDrag();
				SetCursor(nullptr);
				SNDMSG(hLB, WM_LBUTTONUP, 0, 0);
			}
			setCurSel(-1);
			ShowWindow(allBoughtButton.getHwnd(), SW_HIDE);
			drawFocus(false);
		}
		break;
	case WM_GETDLGCODE:
		switch (wParam)
		{
		case VK_RETURN:
			return DLGC_WANTALLKEYS;
		}
		break;
	case WM_NCPAINT:
		{
			if (clock() > animationEndTime)
			{
				if (GetFocus() != hLB && getCurSel() == -1)
				{
					break;
				}
				HDC hDC = GetDCEx(hLB, nullptr, DCX_WINDOW | DCX_PARENTCLIP);
				RECT rcFocus = { rcLB.right - rcLB.left - 2, 0, rcLB.right - rcLB.left, rcLB.bottom - rcLB.top };
				FillRect(hDC, &rcFocus, GetSysColorBrush(COLOR_HIGHLIGHT));
				ReleaseDC(hLB, hDC);
			}
			SetWindowRgn(hLB, CreateRectRgn(0, 0, rcLB.right - rcLB.left - 2, rcLB.bottom - rcLB.top), FALSE);
			LRESULT result = DefSubclassProc(hLB, msg, wParam, lParam);
			SetWindowRgn(hLB, nullptr, FALSE);
			return result;
		}
	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_PRIOR:
		case VK_NEXT:
		case VK_END:
		case VK_HOME:
			if (isEmpty())
			{
				return 0;
			}
			beginScroll();
			break;
		case VK_LEFT:
		case VK_UP:
			{
				int curSel = getCurSel();
				if (isEmpty() || curSel == 2 || curSel == 5 && betsSize == 0)
				{
					return 0;
				}
				if (curSel == 5 + betsSize)
				{
					setCurSel(1 + betsSize);
					return 0;
				}
				beginScroll();
				break;
			}
		case VK_RIGHT:
		case VK_DOWN:
			{
				int curSel = getCurSel();
				if (isEmpty())
				{
					return 0;
				}
				if (curSel == 1 + betsSize)
				{
					if (bankersSize > 0)
					{
						setCurSel(5 + betsSize);
					}
					return 0;
				}
				beginScroll();
				break;
			}
		case VK_DELETE:
			if (getCurSel() > 0)
			{
				SNDMSG(GetParent(hLB), WM_COMMAND, ID_DELETE, 0);
			}
			return 0;
		case VK_RETURN:
			if (IsWindowVisible(allBoughtButton.getHwnd()))
			{
				showChangeBoughtEdit();
			}
			return 0;
		}
		break;
	case WM_LBUTTONDBLCLK:
		if (IsWindowVisible(allBoughtButton.getHwnd()) && GET_Y_LPARAM(lParam) <= (betsSize + bankersSize + 5) * listItemHeight)
		{
			showChangeBoughtEdit();
		}
		return 0;
	case WM_CONTEXTMENU:
		{
			int curSel = getCurSel();
			if (curSel < 0 || IsWindowVisible(boughtEdit.getHwnd()))
			{
				return 0;
			}
			RECT rect;
			ListBox_GetItemRect(hLB, curSel, &rect);
			MapWindowRect(hLB, HWND_DESKTOP, &rect);
			POINT pos = { lParam == -1 ? (rect.left + rect.right) / 2 : GET_X_LPARAM(lParam),lParam == -1 ? (rect.top + rect.bottom) / 2 : GET_Y_LPARAM(lParam) };
			if (!PtInRect(&rect, pos))
			{
				return 0;
			}
			HMENU menu = CreatePopupMenu();
			AppendMenu(menu, 0, ID_DELETE, _T("删除(&D)"));
			TrackPopupMenu(menu, 0, pos.x, pos.y, 0, GetParent(hLB), nullptr);
			DestroyMenu(menu);
			return 0;
		}
	case WM_VSCROLL:
	case WM_MOUSEWHEEL:
		{
			int curSel = getCurSel();
			if (curSel > betsSize + 3)
			{
				beginScroll();
				LRESULT result = DefSubclassProc(hLB, msg, wParam, lParam);
				if (!IsWindowVisible(boughtEdit.getHwnd()))
				{
					showAllBoughtButton(curSel);
				}
				else
				{
					RECT rcSel;
					ListBox_GetItemRect(hLB, curSel, &rcSel);
					MapWindowRect(hLB, GetParent(hLB), &rcSel);
					if (rcSel.top > rcLB.top && rcSel.bottom < rcLB.bottom)
					{
						SetWindowPos(boughtEdit.getHwnd(), HWND_TOP, rcSel.right - listItemHeight + X_CHANGE, rcSel.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
					}
					else
					{
						ShowWindow(boughtEdit.getHwnd(), SW_HIDE);
						SetFocus(hLB);
					}
				}
				endScroll();
				return result;
			}
			break;
		}
	}

	LRESULT result = ListBox::wndProc(msg, wParam, lParam);

	switch (msg)
	{
	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_PRIOR:
		case VK_NEXT:
		case VK_END:
		case VK_HOME:
		case VK_LEFT:
		case VK_UP:
		case VK_RIGHT:
		case VK_DOWN:
			{
				int curSel = getCurSel();
				if (curSel == -1)
				{
					curSel = ListBox_GetCurSel(hLB);
					if (betsSize == 0)
					{
						curSel = 5;
					}
					else if (curSel < 2)
					{
						curSel = 2;
					}
					else if (bankersSize == 0 || curSel < betsSize + 3)
					{
						curSel = betsSize + 1;
					}
					else
					{
						curSel = betsSize + 5;
					}
					setCurSel(curSel);
				}
				if (isScrolling)
				{
					endScroll();
					if (curSel > betsSize + 3)
					{
						showAllBoughtButton(curSel);
					}
				}
				break;
			}
		}
		break;
	}
	return result;
}

void BetList::drawItem(PDRAWITEMSTRUCT pDrawItemStruct)
{
	HDC hDC = pDrawItemStruct->hDC;
	UINT itemID = pDrawItemStruct->itemID;
	UINT itemState = pDrawItemStruct->itemState;
	RECT& rcItem = pDrawItemStruct->rcItem;
	if ((itemID < 2 || betsSize + 1 < itemID && itemID < betsSize + 5) && itemState & ODS_SELECTED)
	{
		itemState -= ODS_SELECTED;
		ShowWindow(allBoughtButton.getHwnd(), SW_HIDE);
	}
	if (hScrollingBm != nullptr && itemState & ODS_SELECTED)
	{
		HDC hScrollingDC = CreateCompatibleDC(hDC);
		HGDIOBJ hOldBm = SelectObject(hScrollingDC, hScrollingBm);
		BitBlt(hDC, rcItem.left, rcItem.top, rcItem.right - rcItem.left, rcItem.bottom - rcItem.top, hScrollingDC, 0, 0, SRCCOPY);
		SelectObject(hScrollingDC, hOldBm);
		DeleteDC(hScrollingDC);
		return;
	}
	HDC hMemDC;
	HPAINTBUFFER hPaintBuffer = BeginBufferedPaint(hDC, &rcItem, BPBF_COMPATIBLEBITMAP, nullptr, &hMemDC);
	if (itemState & ODS_SELECTED)
	{
		FillRect(hMemDC, &rcItem, GetSysColorBrush(COLOR_HIGHLIGHT));
		int posY = rcLB.top + rcItem.top + 2;
		if (!IsWindowVisible(boughtEdit.getHwnd()))
		{
			if (itemID > betsSize + 3)
			{
				if (isScrolling)
				{
					RECT rcAllBoughtButton = { rcItem.right - listItemHeight,rcItem.top,rcItem.right,rcItem.bottom };
					allBoughtButton.drawButton(hMemDC, PBS_NORMAL, rcAllBoughtButton);
				}
				else
				{
					if (rcItem.top >= 0 && rcItem.bottom <= rcLB.bottom - rcLB.top)
					{
						SetWindowPos(allBoughtButton.getHwnd(), HWND_TOP, rcLB.left + rcItem.right - listItemHeight + 2, posY, listItemHeight, listItemHeight, SWP_SHOWWINDOW);
					}
					else
					{
						ShowWindow(allBoughtButton.getHwnd(), SW_HIDE);
					}
				}
			}
			else
			{
				ShowWindow(allBoughtButton.getHwnd(), SW_HIDE);
			}
		}
	}
	else
	{
		FillRect(hMemDC, &rcItem, GetSysColorBrush(COLOR_WINDOW));
	}
	if (itemID == betsSize + 2)
	{
		rcItem.top += (rcItem.bottom - rcItem.top) / 2;
		rcItem.bottom = rcItem.top + 1;
		FillRect(hMemDC, &rcItem, GetSysColorBrush(COLOR_WINDOWTEXT));
	}
	else
	{
		if (itemID > betsSize + 4)
		{
			rcItem.right -= listItemHeight + xScale;
		}
		SetBkMode(hMemDC, TRANSPARENT);
		COLORREF color = pDrawItemStruct->itemData >> 8;
		if (color == 0)
		{
			color = GetSysColor(COLOR_WINDOWTEXT);
		}
		SetTextColor(hMemDC, itemState & ODS_SELECTED ? ::GetSysColor(COLOR_HIGHLIGHTTEXT) : color);
		HFONT hOldFont = SelectFont(hMemDC, itemID == 0 || itemID == betsSize + 3 ? hBoldFont : hFont);
		TCHAR sText[30];
		ListBox_GetText(hLB, itemID, sText);
		BYTE style = pDrawItemStruct->itemData & 0xff;
		DrawText(hMemDC, sText, -1, &rcItem, style);
		SelectFont(hMemDC, hOldFont);
	}
	EndBufferedPaint(hPaintBuffer, TRUE);
}

BOOL BetList::beginDrag(POINT ptCursor)
{
	int dragIdx = LBItemFromPt(hLB, ptCursor, FALSE);
	return dragIdx > 1 && dragIdx < 2 + betsSize && betsSize > 1 || dragIdx > betsSize + 4 && bankersSize > 1;
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
		if (curSel < betsSize + 3)
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
		SetWindowRedraw(hLB, FALSE);
		ListBox_DeleteString(hLB, curSel);
		if (curSel < lastDragIdx)
		{
			lastDragIdx--;
		}
		ListBox_InsertString(hLB, lastDragIdx, str);
		ListBox_SetItemData(hLB, lastDragIdx, itemData);
		setCurSel(lastDragIdx);
		SetWindowRedraw(hLB, TRUE);
	}
	if (curSel > betsSize + 3)
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
	if (getCurSel() > betsSize + 3)
	{
		ListBox_DeleteString(hLB, betsSize + bankersSize + 5);
		if (GetScrollPos(hLB, SB_VERT) >= betsSize + bankersSize + 5 - maxDisplayedItemCnt)
		{
			ListBox_SetTopIndex(hLB, betsSize + bankersSize + 5 - maxDisplayedItemCnt);
		}
	}
}

int BetList::getBetsSize() const
{
	return betsSize;
}

int BetList::getBankersSize() const
{
	return bankersSize;
}

int BetList::getCurSel() const
{
	int curSel = ListBox_GetCurSel(hLB);
	if (curSel < 2 || betsSize + 1 < curSel && curSel < betsSize + 5)
	{
		return -1;
	}
	return curSel;
}

void BetList::addBet(double odds, int amount)
{
	TCHAR str[13];
	_stprintf(str, _T("%0.1f  %" STR(MAX_BET_AMOUNT_LEN) "d"), odds, amount);
	insertString(betsSize + 2, str, DT_CENTER);
	betsSize++;
	ListBox_SetTopIndex(hLB, betsSize > maxDisplayedItemCnt - 2 ? betsSize + 2 - maxDisplayedItemCnt : 0);
}

void BetList::addBanker(const Banker& banker)
{
	TCHAR str[20];
	_stprintf(str, _T("%0.1f %" STR(MAX_BET_AMOUNT_LEN) "d       0"), banker.odds, banker.amount);
	addString(str, DT_RIGHT, RGB(255, 0, 0));
	bankersSize++;
	ListBox_SetTopIndex(hLB, betsSize + bankersSize + 6 - maxDisplayedItemCnt);
}

void BetList::updateBanker(int nIndex, const Banker& banker)
{
	TCHAR str[20];
	_stprintf(str, _T("%0.1f %" STR(MAX_BET_AMOUNT_LEN) "d %" STR(MAX_BET_AMOUNT_LEN) "d"), banker.odds, banker.amount, banker.bought);
	SetWindowRedraw(hLB, FALSE);
	ShowWindow(allBoughtButton.getHwnd(), SW_HIDE);
	insertString(nIndex, str, DT_RIGHT, banker.maxBought == banker.bought ? GetSysColor(COLOR_WINDOWTEXT) : RGB(255, 0, 0));
	ListBox_DeleteString(hLB, nIndex + 1);
	SetWindowRedraw(hLB, TRUE);
}

pair<bool, int> BetList::deleteSel()
{
	int lineIdx = getCurSel();
	pair<bool, int> result;
	result.first = lineIdx < betsSize + 3;
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
	ShowWindow(allBoughtButton.getHwnd(), SW_HIDE);
	ShowWindow(boughtEdit.getHwnd(), SW_HIDE);
	return result;
}

bool BetList::isEmpty() const
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

void BetList::addString(PCTSTR str, BYTE style, COLORREF color)
{
	int index = ListBox_AddString(hLB, str);
	ListBox_SetItemData(hLB, index, ((color << 8) | style));
}

void BetList::insertString(int nIndex, PCTSTR str, BYTE style, COLORREF color)
{
	int index = ListBox_InsertString(hLB, nIndex, str);
	ListBox_SetItemData(hLB, index, ((color << 8) | style));
}

void BetList::showAllBoughtButton(int nIndex)
{
	RECT rcSel;
	ListBox_GetItemRect(hLB, nIndex, &rcSel);
	MapWindowRect(hLB, GetParent(hLB), &rcSel);
	if (rcSel.top > rcLB.top && rcSel.bottom < rcLB.bottom)
	{
		SetWindowPos(allBoughtButton.getHwnd(), nullptr, rcSel.right - listItemHeight, rcSel.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW);
	}
	else
	{
		ShowWindow(allBoughtButton.getHwnd(), SW_HIDE);
	}
}

void BetList::showChangeBoughtEdit()
{
	int curSel = ListBox_GetCurSel(hLB);
	ShowWindow(allBoughtButton.getHwnd(), SW_HIDE);
	RECT rcSel;
	ListBox_GetItemRect(hLB, curSel, &rcSel);
	MapWindowRect(hLB, GetParent(hLB), &rcSel);
	ShowWindow(allBoughtButton.getHwnd(), SW_HIDE);
	SetWindowPos(boughtEdit.getHwnd(), HWND_TOP, rcSel.right - listItemHeight + X_CHANGE, rcSel.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW);
	TCHAR str[20];
	ListBox_GetText(hLB, curSel, str);
	int i;
	for (i = 12; str[i] == ' '; i++);
	boughtEdit.setText(&str[i], false);
	boughtEdit.setSel(0, -1);
	SetFocus(boughtEdit.getHwnd());
}

void BetList::beginScroll()
{
	int curSel = getCurSel();
	if (isScrolling || curSel < betsSize + 3)
	{
		return;
	}
	isScrolling = true;
	RECT rcSel;
	ListBox_GetItemRect(hLB, curSel, &rcSel);
	if (rcSel.top<0 || rcSel.bottom>rcLB.bottom - rcLB.top)
	{
		return;
	}
	SetWindowRgn(allBoughtButton.getHwnd(), CreateRectRgn(0, 0, 0, 0), FALSE);
	SetWindowRgn(boughtEdit.getHwnd(), CreateRectRgn(0, 0, 0, 0), FALSE);
	HDC hLBDC = GetDC(hLB);
	HDC hScrollingDC = CreateCompatibleDC(hLBDC);
	hScrollingBm = CreateCompatibleBitmap(hLBDC, rcSel.right - rcSel.left, rcSel.bottom - rcSel.top);
	HGDIOBJ hOldBm = SelectObject(hScrollingDC, hScrollingBm);
	BitBlt(hScrollingDC, 0, 0, rcSel.right - rcSel.left, rcSel.bottom - rcSel.top, hLBDC, rcSel.left, rcSel.top, SRCCOPY);
	SelectObject(hScrollingDC, hOldBm);
	DeleteDC(hScrollingDC);
	ReleaseDC(hLB, hLBDC);
}

void BetList::endScroll()
{
	SetWindowRgn(allBoughtButton.getHwnd(), nullptr, TRUE);
	SetWindowRgn(boughtEdit.getHwnd(), nullptr, TRUE);
	isScrolling = false;
	DeleteObject(hScrollingBm);
	hScrollingBm = nullptr;
}

void BetList::eraseDragLine(HDC hDC)
{
	RECT rcLastLine;
	ListBox_GetItemRect(hLB, lastDragIdx, &rcLastLine);
	rcLastLine.top--;
	rcLastLine.bottom = rcLastLine.top + 2;
	FillRect(hDC, &rcLastLine, GetSysColorBrush(COLOR_WINDOW));
}


// class ResultList

LRESULT ResultList::wndProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_SETFOCUS:
		drawFocus(true);
		break;
	case WM_KILLFOCUS:
		drawFocus(false);
		break;
	case WM_NCPAINT:
		{
			if (clock() > animationEndTime)
			{
				if (GetFocus() != hLB)
				{
					break;
				}
				HDC hDC = GetDCEx(hLB, nullptr, DCX_WINDOW | DCX_PARENTCLIP);
				RECT rcFocus = { rcLB.right - rcLB.left - 2, 0, rcLB.right - rcLB.left, rcLB.bottom - rcLB.top };
				FillRect(hDC, &rcFocus, GetSysColorBrush(COLOR_HIGHLIGHT));
				ReleaseDC(hLB, hDC);
			}
			SetWindowRgn(hLB, CreateRectRgn(0, 0, rcLB.right - rcLB.left - 2, rcLB.bottom - rcLB.top), FALSE);
			LRESULT result = DefSubclassProc(hLB, msg, wParam, lParam);
			SetWindowRgn(hLB, nullptr, FALSE);
			return result;
		}
	case WM_CONTEXTMENU:
		{
			int curSel = ListBox_GetCurSel(hLB);
			RECT rect;
			ListBox_GetItemRect(hLB, curSel, &rect);
			MapWindowRect(hLB, HWND_DESKTOP, &rect);
			POINT pos = lParam == -1 ? POINT((rect.left + rect.right) / 2, (rect.top + rect.bottom) / 2) : POINT(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
			if (!PtInRect(&rect, pos))
			{
				return 0;
			}
			HMENU menu = CreatePopupMenu();
			AppendMenu(menu, 0, 1, _T("复制投入数量(&C)"));
			if (TrackPopupMenu(menu, TPM_NONOTIFY | TPM_RETURNCMD, pos.x, pos.y, 0, hLB, nullptr) == 1)
			{
				__int64 aimAmount = ListBox_GetItemData(hLB, curSel);
				HANDLE hStr = GlobalAlloc(GMEM_MOVEABLE, (MAX_INITIAL_AMOUNT_LEN + 1) * sizeof(TCHAR));
				_i64tot(aimAmount, (PWSTR)GlobalLock(hStr), 10);
				GlobalUnlock(hStr);
				OpenClipboard(hLB);
				EmptyClipboard();
				SetClipboardData(CF_UNICODETEXT, hStr);
				CloseClipboard();
			}
			DestroyMenu(menu);
			return 0;
		}
	}

	LRESULT result = ListBox::wndProc(msg, wParam, lParam);

	switch (msg)
	{
	}
	return result;
}

void ResultList::drawItem(PDRAWITEMSTRUCT pDrawItemStruct)
{
	HDC hMemDC;
	HPAINTBUFFER hPaintBuffer = BeginBufferedPaint(pDrawItemStruct->hDC, &pDrawItemStruct->rcItem, BPBF_COMPATIBLEBITMAP, nullptr, &hMemDC);
	HFONT hOldFont = SelectFont(hMemDC, hFont);
	FillRect(hMemDC, &pDrawItemStruct->rcItem, GetSysColorBrush(pDrawItemStruct->itemState & ODS_SELECTED ? COLOR_HIGHLIGHT : COLOR_WINDOW));

	SetBkMode(hMemDC, TRANSPARENT);
	SetTextColor(hMemDC, pDrawItemStruct->itemState & ODS_SELECTED ? ::GetSysColor(COLOR_HIGHLIGHTTEXT) : GetSysColor(COLOR_WINDOWTEXT));
	TCHAR sText[30];
	ListBox_GetText(hLB, pDrawItemStruct->itemID, sText);
	DrawText(hMemDC, sText, -1, &pDrawItemStruct->rcItem, DT_SINGLELINE);
	SelectFont(hMemDC, hOldFont);
	EndBufferedPaint(hPaintBuffer, TRUE);
}

int ResultList::addResult(PCTSTR str, __int64 aimAmount)
{
	int index = ListBox_AddString(hLB, str);
	ListBox_SetItemData(hLB, index, aimAmount);
	return index;
}
