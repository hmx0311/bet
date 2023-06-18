#pragma once
#include "Dialog.h"

#include "edit.h"
#include "common.h"

class ConfigDlg :
	public Dialog
{
private:
	HWND hDefCutCombo;
	NumericEdit defCutEdit;
	HWND hDefClosingCheck;
	NumericEdit fastAddedAmountEdit[4];
	HWND hCurrentAmountCfgCombo;
	NumericEdit defProbErrorEdit;
public:
	ConfigDlg();
	virtual INT_PTR initDlg(HWND hDlg);
	virtual INT_PTR dlgProc(UINT msg, WPARAM wParam, LPARAM lParam);
private:
	void setCaptionFont();
};