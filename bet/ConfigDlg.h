#pragma once
#include "Dialog.h"

#include "edit.h"
#include "common.h"

class ConfigDlg :
	public Dialog
{
private:
	HWND hDefaultCutCombo;
	NumericEdit defaultCutEdit;
	HWND hDefaultClosingCheck;
	NumericEdit fastAddedAmountEdit[4];
	NumericEdit defaultProbErrorEdit;
public:
	ConfigDlg();
	virtual INT_PTR initDlg(HWND hDlg);
	virtual INT_PTR dlgProc(UINT msg, WPARAM wParam, LPARAM lParam);
};