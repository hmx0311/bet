#pragma once

#include "bet.h"

void setVCentered(HWND hEdit);
LRESULT CALLBACK editSubclassProc(HWND hEdit, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);

class Edit
{
protected:
	HWND hEdit;
public:
	void attach(HWND hListBox);
	virtual LRESULT wndProc(UINT msg, WPARAM wParam, LPARAM lParam) = 0;
	HWND getHwnd();
	void getRect(RECT* rect);
	void setRectNP(RECT* rect);
	void setTextLimit(int limit);
	void setText(LPCWSTR str);
	void getText(LPWSTR str, int nMaxCount);
	void setSel(int start, int end);
};