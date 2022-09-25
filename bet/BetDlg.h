#pragma once
#include "Dialog.h"
#include "BetTabDlg.h"

class BetDlg :
	public Dialog
{
public:
	static HICON hIcon;
private:
	std::vector<Dialog*> betTabs;
	Dialog* currentDlg;

	HWND hBetTab;
	Button settingsButton;
	Button addTabButton;
	HWND hTabNameEdit;

	int lastSel = -1;
	bool needErase = true;

public:
	BetDlg();
	virtual INT_PTR initDlg(HWND hDlg);
	virtual INT_PTR dlgProc(UINT msg, WPARAM wParam, LPARAM lParam);
	Dialog* getCurrentTab();
private:
	void createBetTabDlg(double cut);
	void createTab();
	void calcPos();
	void calcBetTabPos();
};

