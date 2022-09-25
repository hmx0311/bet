#pragma once
#include "Dialog.h"
#include "edit.h"
class CutInputDlg :
    public Dialog
{
private:
    NumericEdit cutEdit;
public:
    CutInputDlg();
    virtual INT_PTR initDlg(HWND hDlg);
    virtual INT_PTR dlgProc(UINT msg, WPARAM wParam, LPARAM lParam);
    double getCut();
};

