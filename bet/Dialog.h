#pragma once
#include "bet.h"

INT_PTR CALLBACK dlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);

class Dialog
{
protected:
	HWND hDlg = nullptr;
	const UINT nIDTemplate;
public:
	Dialog(UINT nIDTemplate);
	virtual INT_PTR initDlg(HWND hDlg);
	virtual INT_PTR dlgProc(UINT msg, WPARAM wParam, LPARAM lParam) = 0;
	void createDialog(HWND hWndParent = nullptr);
	INT_PTR dialogBox(HWND hWndParent = nullptr);
	HWND getHwnd();
};