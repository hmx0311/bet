#pragma once

#include "ListBox.h"
#include "NumericEdit.h"

#include <utility>

class BetList :
	public ListBox
{
public:
	static int maxDisplayedItemCnt;
private:
	HWND hMoveSpin;
	HWND hAllBoughtButton;
	NumericEdit* boughtEdit;
	int betsSize = 0;
	int bankersSize = 0;
	RECT rcListBox;

public:
	virtual LRESULT wndProc(UINT message, WPARAM wParam, LPARAM lParam);
	virtual void drawItem(HDC hDC, int itemID, UINT itemState, ULONG_PTR itemData, RECT& rcItem);
	void attach(HWND hListBox, HWND hMoveSpin, HWND hAllBoughtButton, NumericEdit* boughtEdit);
	int getBetsSize();
	int getBankersSize();
	int getCurSel();
	int getItemRect(int nIndex, LPRECT lpRect);
	int getScrollPos();
	void setTopIndex(int nIndex);
	void addBet(LPCTSTR str);
	void addBanker(LPCTSTR str);
	void updateBanker(int nIndex, LPCTSTR lpszItem, COLORREF color = RGB(0, 0, 0));
	int insertString(int nIndex, LPCTSTR lpszItem, BYTE style, COLORREF color = RGB(0, 0, 0));
	int moveSel(bool direction);
	std::pair<bool, int> deleteSel();
	bool isEmpty();
	void resetContent();
};

