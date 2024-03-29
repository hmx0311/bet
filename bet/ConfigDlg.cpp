#include "framework.h"
#include "ConfigDlg.h"

#include "common.h"
#include "controls.h"
#include "button.h"

#include <fstream>
#include <windowsx.h>

using namespace std;

HFONT hCfgDlgBoldFont;

ConfigDlg::ConfigDlg()
	:Dialog(IDD_CONFIG_DIALOG) {}

INT_PTR ConfigDlg::initDlg(HWND hDlg)
{
	Dialog::initDlg(hDlg);
	SetDialogDpiChangeBehavior(hDlg, DDC_DISABLE_RESIZE, DDC_DISABLE_RESIZE);

	hDefCutCombo = GetDlgItem(hDlg, IDC_DEF_CUT_COMBO);
	SNDMSG(hDefCutCombo, WM_UPDATEUISTATE, MAKEWPARAM(UIS_SET, UISF_HIDEFOCUS), 0);
	SetWindowSubclass(hDefCutCombo, noFocusRectSubclassProc, 0, 0);
	defCutEdit.attach(GetDlgItem(hDlg, IDC_DEF_CUT_EDIT));
	hDefClosingCheck = GetDlgItem(hDlg, IDC_DEF_CLOSING_CHECK);
	SNDMSG(hDefClosingCheck, WM_UPDATEUISTATE, MAKEWPARAM(UIS_SET, UISF_HIDEFOCUS), 0);
	SetWindowSubclass(hDefClosingCheck, buttonSubclassProc, 0, 0);
	for (int i = 0; i < 4; i++)
	{
		fastAddedAmountEdit[i].attach(GetDlgItem(hDlg, IDC_FAST_ADDED_AMOUNT_EDIT1 + i));
	}
	defProbErrorEdit.attach(GetDlgItem(hDlg, IDC_DEF_PROB_ERROR_EDIT));
	hCurrentAmountCfgCombo = GetDlgItem(hDlg, IDC_CURRENT_AMOUNT_CFG_COMBO);
	SNDMSG(hCurrentAmountCfgCombo, WM_UPDATEUISTATE, MAKEWPARAM(UIS_SET, UISF_HIDEFOCUS), 0);
	SetWindowSubclass(hCurrentAmountCfgCombo, noFocusRectSubclassProc, 0, 0);
	SNDMSG(GetDlgItem(hDlg, IDOK), WM_UPDATEUISTATE, MAKEWPARAM(UIS_SET, UISF_HIDEFOCUS), 0);
	SetWindowSubclass(GetDlgItem(hDlg, IDOK), noFocusRectSubclassProc, 0, 0);
	SNDMSG(GetDlgItem(hDlg, IDCANCEL), WM_UPDATEUISTATE, MAKEWPARAM(UIS_SET, UISF_HIDEFOCUS), 0);
	SetWindowSubclass(GetDlgItem(hDlg, IDCANCEL), noFocusRectSubclassProc, 0, 0);

	setCaptionFont();

	ImmAssociateContext(hDefCutCombo, nullptr);
	ComboBox_AddString(hDefCutCombo, _T("新建时输入"));
	ComboBox_AddString(hDefCutCombo, _T("使用默认值"));
	ComboBox_SetCurSel(hDefCutCombo, config.useDefCut);
	TCHAR str[6];
	_stprintf(str, _T("%04d"), lround((1 - config.defCut) * 10000));
	defCutEdit.setText(str, false);
	Button_SetCheck(hDefClosingCheck, config.defClosing);
	for (int i = 0; i < 4; i++)
	{
		_itot(config.fastAddedAmount[i], str, 10);
		fastAddedAmountEdit[i].setText(str, false);
	}
	ImmAssociateContext(hCurrentAmountCfgCombo, nullptr);
	ComboBox_AddString(hCurrentAmountCfgCombo, _T("初始数量"));
	ComboBox_AddString(hCurrentAmountCfgCombo, _T("剩余数量"));
	ComboBox_SetCurSel(hCurrentAmountCfgCombo, config.currentAmountDisplayMode);
	_itot(lround(config.defProbError * 100), str, 10);
	defProbErrorEdit.setText(str, false);

	return (INT_PTR)TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

INT_PTR ConfigDlg::dlgProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_DPICHANGED:
		{
			DeleteObject(hCfgDlgBoldFont);
			setCaptionFont();
			RECT* rect = (RECT*)lParam;
			MoveWindow(hDlg, rect->left, rect->top, rect->right - rect->left, rect->bottom - rect->top, TRUE);
		}
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_DEF_CUT_COMBO:
			switch (HIWORD(wParam))
			{
			case CBN_CLOSEUP:
				SetFocus(GetDlgItem(hDlg, IDOK));
				return (INT_PTR)TRUE;
			}
			break;
		case IDOK:
			{
				config.useDefCut = ComboBox_GetCurSel(hDefCutCombo);
				config.defCut = 0;
				TCHAR str[6];
				defCutEdit.getText(str, 6);
				for (int i = lstrlen(str) - 1; i >= 0; i--)
				{
					int c = str[i] - '0';
					config.defCut = (config.defCut + c) * 0.1;
				}
				config.defCut = 1 - config.defCut;
				config.defClosing = Button_GetCheck(hDefClosingCheck);
				for (int i = 0; i < 4; i++)
				{
					fastAddedAmountEdit[i].getText(str, 6);
					int amount = _ttoi(str);
					if (amount == 0)
					{
						SetFocus(fastAddedAmountEdit[i].getHwnd());
						fastAddedAmountEdit[i].setSel(0, -1);
						return (INT_PTR)TRUE;
					}
					config.fastAddedAmount[i] = amount;
				}
				config.currentAmountDisplayMode = ComboBox_GetCurSel(hCurrentAmountCfgCombo);
				defProbErrorEdit.getText(str, 6);
				config.defProbError = _ttoi(str) / 100.0;
				ofstream file(CONFIG_FILE_NAME, ios::out | ios::binary);
				if (file.good())
				{
					file.write((char*)&config, sizeof(Config));
					file.close();
				}
				MessageBox(hDlg, _T("设置已更改，将应用于新竞猜"), _T("bet设置"), MB_OK | MB_ICONINFORMATION);
			}
		case IDCANCEL:
			EndDialog(hDlg, 0);
			break;
		}
	}
	return (INT_PTR)FALSE;
}

void ConfigDlg::setCaptionFont()
{
	hCfgDlgBoldFont = createBoldFont(GetWindowFont(hDlg));
	SetWindowFont(GetDlgItem(hDlg, IDC_GENERAL_CFG_CAPTION), hCfgDlgBoldFont, FALSE);
	SetWindowFont(GetDlgItem(hDlg, IDC_WIN_PROB_MODE_CFG_CAPTION), hCfgDlgBoldFont, FALSE);
}