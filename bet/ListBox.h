#pragma once

#include "edit.h"
#include "model.h"

#include <time.h>
#include <utility>

class ListBox
{
protected:
	HWND hLB;
	RECT rcLB;
	int maxDisplayedItemCnt;
	clock_t animationEndTime = 0;
public:
	void attach(HWND hLB);
	virtual LRESULT wndProc(UINT msg, WPARAM wParam, LPARAM lParam);
	virtual void drawItem(PDRAWITEMSTRUCT pDrawItemStruct) = 0;
	HWND getHwnd() const;
	void setCurSel(int nSelect);
protected:
	void drawFocus(bool isFocused);
};

class BetList :
	public ListBox
{
private:
	Button& allBoughtButton;
	NumericEdit& boughtEdit;
	bool isScrolling = false;
	HBITMAP hScrollingBm = nullptr;
	bool isDragging = false;
	int lastDragIdx = -1;
	int betsSize = 0;
	int bankersSize = 0;

public:
	BetList(Button& allBoughtButton, NumericEdit& boughtEdit);
	void attach(HWND hLB);
	virtual LRESULT wndProc(UINT msg, WPARAM wParam, LPARAM lParam);
	virtual void drawItem(PDRAWITEMSTRUCT pDrawItemStruct);
	BOOL beginDrag(POINT ptCursor);
	UINT dragging(POINT ptCursor);
	int dropped(POINT ptCursor);
	void cancelDrag();
	int getBetsSize() const;
	int getBankersSize() const;
	int getCurSel() const;
	void addBet(double odds, int amount);
	void addBanker(const Banker& banker);
	void updateBanker(int nIndex, const Banker& banker);
	std::pair<bool, int> deleteSel();
	bool isEmpty() const;
	void resetContent();
private:
	void addString(PCTSTR str, BYTE style = 0, COLORREF color = 0);
	void insertString(int nIndex, PCTSTR str, BYTE style, COLORREF color = 0);
	void showAllBoughtButton(int nIndex);
	void showChangeBoughtEdit();
	void beginScroll();
	void endScroll();
	void eraseDragLine(HDC hDC);
};

class ResultList :
	public ListBox
{
public:
	virtual LRESULT wndProc(UINT msg, WPARAM wParam, LPARAM lParam);
	virtual void drawItem(PDRAWITEMSTRUCT pDrawItemStruct);
	int addResult(PCTSTR str, __int64 aimAmount);
};