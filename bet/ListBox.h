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
	int addString(PCTSTR pszItem, BYTE style = 0, COLORREF color = 0);
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
	RECT rcListBox;
	HWND hMoveSpin;
	HWND hAllBoughtButton;
	NumericEdit* boughtEdit;
	int betsSize = 0;
	int bankersSize = 0;

public:
	void attach(HWND hListBox, HWND hMoveSpin, HWND hAllBoughtButton, NumericEdit* boughtEdit);
	virtual LRESULT wndProc(UINT msg, WPARAM wParam, LPARAM lParam);
	virtual void drawItem(HDC hDC, int itemID, UINT itemState, ULONG_PTR itemData, RECT& rcItem);
	int getBetsSize();
	int getBankersSize();
	int getCurSel();
	void addBet(PCTSTR str);
	void addBanker(PCTSTR str);
	void updateBanker(int nIndex, PCTSTR pszItem, COLORREF color = 0);
	int moveSel(bool direction);
	std::pair<bool, int> deleteSel();
	bool isEmpty();
	void resetContent();
private:
	int insertString(int nIndex, PCTSTR pszItem, BYTE style, COLORREF color = 0);
	int getItemRect(int nIndex, PRECT pRect);
	void setTopIndex(int nIndex);
	int getScrollPos();
};