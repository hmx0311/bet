#pragma once
#include "Dialog.h"

#include "CheckBox.h"
#include "NumericEdit.h"
#include "common.h"

class ConfigDlg :
    public Dialog
{
private:
	Config oldConfig;
	NumericEdit cutEdit;
	CheckBox defaultClosingCheck;
	NumericEdit fastAddedAmountEdit[4];
	NumericEdit defaultProbErrorEdit;
public:
	ConfigDlg();
	virtual INT_PTR initDlg(HWND hDlg);
	virtual INT_PTR dlgProc(UINT message, WPARAM wParam, LPARAM lParam);
};