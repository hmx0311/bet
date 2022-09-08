#pragma once

#include "edit.h"

#include <utility>

class ListBox
{
protected:
	HWND hListBox;
public:
	void attach(HWND hListBox);
	virtual LRESULT wndProc(UINT msg, WPARAM wParam, LPARAM lParam);
	virtual void drawItem(HDC hDC, int itemID, UINT itemState, ULONG_PTR itemData, RECT& rcItem);
	HWND getHwnd();
	int addString(PCTSTR lpszItem, BYTE style = 0, COLORREF color = GetSysColor(COLOR_WINDOWTEXT));
	void getText(int nIndex, PTSTR str);
	void setCurSel(int nSelect);
	int getCurSel();
};

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
	void attach(HWND hListBox, HWND hMoveSpin, HWND hAllBoughtButton, NumericEdit* boughtEdit);
	virtual LRESULT wndProc(UINT msg, WPARAM wParam, LPARAM lParam);
	virtual void drawItem(HDC hDC, int itemID, UINT itemState, ULONG_PTR itemData, RECT& rcItem);
	int getBetsSize();
	int getBankersSize();
	int getCurSel();
	void addBet(PCTSTR str);
	void addBanker(PCTSTR str);
	void updateBanker(int nIndex, PCTSTR lpszItem, COLORREF color = GetSysColor(COLOR_WINDOWTEXT));
	int moveSel(bool direction);
	std::pair<bool, int> deleteSel();
	bool isEmpty();
	void resetContent();
private:
	int insertString(int nIndex, PCTSTR lpszItem, BYTE style, COLORREF color = GetSysColor(COLOR_WINDOWTEXT));
	int getItemRect(int nIndex, PRECT lpRect);
	void setTopIndex(int nIndex);
	int getScrollPos();
};