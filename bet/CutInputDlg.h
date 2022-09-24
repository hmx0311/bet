#pragma once
#include "Dialog.h"
#include "edit.h"
class CutInputDlg :
    public Dialog
{
private:
    NumericEdit cutEdit;
    double& cut;
public:
    CutInputDlg(double& cut);
    virtual INT_PTR initDlg(HWND hDlg);
    virtual INT_PTR dlgProc(UINT msg, WPARAM wParam, LPARAM lParam);
};

