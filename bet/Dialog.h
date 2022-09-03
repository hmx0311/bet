#pragma once
#include "bet.h"

INT_PTR CALLBACK dlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

class Dialog
{
protected:
	HWND hDlg = nullptr;
	UINT nIDTemplate;
public:
	virtual INT_PTR initDlg(HWND hDlg);
	virtual INT_PTR dlgProc(UINT message, WPARAM wParam, LPARAM lParam) = 0;
	void createDialog(HWND hWndParent = nullptr);
	INT_PTR dialogBox(HWND hWndParent = nullptr);
	HWND getHwnd();
};

