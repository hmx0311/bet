#pragma once

LRESULT CALLBACK listBoxProc(HWND, UINT, WPARAM, LPARAM);

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
