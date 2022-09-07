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
	int addString(LPCTSTR lpszItem, BYTE style = 0, COLORREF color = RGB(0, 0, 0));
	void getText(int nIndex, LPWSTR str);
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
	void addBet(LPCTSTR str);
	void addBanker(LPCTSTR str);
	void updateBanker(int nIndex, LPCTSTR lpszItem, COLORREF color = RGB(0, 0, 0));
	int moveSel(bool direction);
	std::pair<bool, int> deleteSel();
	bool isEmpty();
	void resetContent();
private:
	int insertString(int nIndex, LPCTSTR lpszItem, BYTE style, COLORREF color = RGB(0, 0, 0));
	int getItemRect(int nIndex, LPRECT lpRect);
	void setTopIndex(int nIndex);
	int getScrollPos();
};