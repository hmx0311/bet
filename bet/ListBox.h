#pragma once

#include "edit.h"
#include "model.h"

#include <utility>

class ListBox
{
protected:
	HWND hLB;
	RECT rcLB;
	int maxDisplayedItemCnt;
public:
	void attach(HWND hLB);
	virtual LRESULT wndProc(UINT msg, WPARAM wParam, LPARAM lParam);
	virtual void drawItem(HDC hDC, int itemID, UINT itemState, ULONG_PTR itemData, RECT& rcItem);
	HWND getHwnd();
	int addString(PCTSTR pszItem, BYTE style = 0, COLORREF color = 0);
	void setCurSel(int nSelect);
protected:
	void onDPIChanged();
	void drawFocus();
};

class BetList :
	public ListBox
{
private:
	HWND hAllBoughtButton;
	NumericEdit* boughtEdit;
	bool isDragging = false;
	int lastDragIdx = -1;
	int betsSize = 0;
	int bankersSize = 0;

public:
	void attach(HWND hLB, HWND hAllBoughtButton, NumericEdit* boughtEdit);
	virtual LRESULT wndProc(UINT msg, WPARAM wParam, LPARAM lParam);
	virtual void drawItem(HDC hDC, int itemID, UINT itemState, ULONG_PTR itemData, RECT& rcItem);
	BOOL beginDrag(POINT ptCursor);
	UINT dragging(POINT ptCursor);
	int dropped(POINT ptCursor);
	void cancelDrag();
	int getBetsSize();
	int getBankersSize();
	int getCurSel();
	void addBet(double odds, int amount);
	void addBanker(const Banker& banker);
	void updateBanker(int nIndex, const Banker& banker);
	std::pair<bool, int> deleteSel();
	bool isEmpty();
	void resetContent();
private:
	void eraseDragLine(HDC hDC);
	int insertString(int nIndex, PCTSTR pszItem, BYTE style, COLORREF color = 0);
};