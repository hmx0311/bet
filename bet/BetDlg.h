#pragma once
#include "Dialog.h"
#include "BetTabDlg.h"

class BetDlg :
	public Dialog
{
public:
	static HICON hIcon;
private:
	std::vector<BetTabDlg*> betTabs;
	BetTabDlg* currentTab;

	HWND betTab;
	Button settingsButton;
	Button addTabButton;
	HWND tabNameEdit;

	int lastSel = -1;
	bool needErase = true;

public:
	BetDlg();
	virtual INT_PTR initDlg(HWND hDlg);
	virtual INT_PTR dlgProc(UINT msg, WPARAM wParam, LPARAM lParam);
	BetTabDlg* getCurrentTab();
private:
	void calcPos();
};

