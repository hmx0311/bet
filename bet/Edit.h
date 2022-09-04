#pragma once

#include "bet.h"

extern WNDPROC defEditProc;

void setVCentered(HWND hEdit);
LRESULT CALLBACK editProc(HWND, UINT, WPARAM, LPARAM);

class Edit
{
protected:
	HWND hEdit;
public:
	virtual LRESULT wndProc(UINT message, WPARAM wParam, LPARAM lParam) = 0;
	void attach(HWND hListBox);
	HWND getHwnd();
	void getRect(RECT* rect);
	void setRectNP(RECT* rect);
	void setTextLimit(int limit);
	void setText(LPCWSTR str);
	void getText(LPWSTR str, int nMaxCount);
	void setSel(int start, int end);
};