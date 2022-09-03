#pragma once

extern WNDPROC defListBoxProc;

LRESULT CALLBACK listBoxProc(HWND, UINT, WPARAM, LPARAM);

class ListBox
{
protected:
	HWND hListBox;
public:
	virtual LRESULT listBoxProc(UINT message, WPARAM wParam, LPARAM lParam);
	virtual void drawItem(HDC hDC, int itemID, UINT itemState, ULONG_PTR itemData, RECT& rcItem);
	void attach(HWND hListBox);
	HWND getHwnd();
	int addString(LPCTSTR lpszItem, BYTE style = 0, COLORREF color = RGB(0, 0, 0));
	void getText(int nIndex, LPWSTR str);
	void setCurSel(int nSelect);
	int getCurSel();
};
