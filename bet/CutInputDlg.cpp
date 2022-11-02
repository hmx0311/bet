#include "framework.h"
#include "CutInputDlg.h"

#include "common.h"
#include "button.h"

CutInputDlg::CutInputDlg()
	:Dialog(IDD_CUT_INPUT_DIALOG) {}

INT_PTR CutInputDlg::initDlg(HWND hDlg)
{
	Dialog::initDlg(hDlg);
	cutEdit.attach(GetDlgItem(hDlg, IDC_CUT_EDIT));
	SendMessage(GetDlgItem(hDlg, IDOK), WM_UPDATEUISTATE, MAKEWPARAM(UIS_SET, UISF_HIDEFOCUS), 0);
	SetWindowSubclass(GetDlgItem(hDlg, IDOK), buttonSubclassProc, 0, 0);
	TCHAR str[5];
	_stprintf(str, _T("%04d"), (int)round((1 - config.defCut) * 10000));
	cutEdit.setText(str);
	return (INT_PTR)TRUE;
}

INT_PTR CutInputDlg::dlgProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case ID_CONFIRM:
		case IDOK:
			DestroyWindow(hDlg);
			hDlg = nullptr;
			break;
		}
	}
	return (INT_PTR)FALSE;
}

double CutInputDlg::getCut()
{
	double cut = 0;
	TCHAR str[6];
	cutEdit.getText(str, 6);
	for (int i = lstrlen(str) - 1; i >= 0; i--)
	{
		int c = str[i] - '0';
		cut = (cut + c) * 0.1;
	}
	return 1 - cut;
}
