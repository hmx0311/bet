#pragma once
#include "Dialog.h"
#include "BetTabDlg.h"

class BetDlg :
	public Dialog
{
public:
	static HICON hIcon;
	static HICON hSettingsIcon;
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
	virtual INT_PTR dlgProc(UINT message, WPARAM wParam, LPARAM lParam);
	BetTabDlg* getCurrentTab();
};

