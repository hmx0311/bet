#include "framework.h"
#include "ConfigDlg.h"

#include "button.h"

#include <cmath>
#include <fstream>
#include <windowsx.h>

using namespace std;

ConfigDlg::ConfigDlg()
{
	nIDTemplate = IDD_CONFIG_DIALOG;
}

INT_PTR ConfigDlg::initDlg(HWND hDlg)
{
	Dialog::initDlg(hDlg);

	hDefaultCutCombo = GetDlgItem(hDlg, IDC_DEFAULT_CUT_COMBO);
	defaultCutEdit.attach(GetDlgItem(hDlg, IDC_DEFAULT_CUT_EDIT));
	hDefaultClosingCheck = GetDlgItem(hDlg, IDC_DEFAULT_CLOSING_CHECK);
	SendMessage(hDefaultClosingCheck, WM_UPDATEUISTATE, MAKEWPARAM(UIS_SET, UISF_HIDEFOCUS), 0);
	SetWindowSubclass(hDefaultClosingCheck, buttonSubclassProc, 0, 0);
	for (int i = 0; i < 4; i++)
	{
		fastAddedAmountEdit[i].attach(GetDlgItem(hDlg, IDC_FAST_ADDED_AMOUNT_EDIT1 + i));
	}
	defaultProbErrorEdit.attach(GetDlgItem(hDlg, IDC_DEFAULT_PROBABILTY_ERROR_EDIT));

	SendMessage(hDefaultCutCombo, WM_UPDATEUISTATE, MAKEWPARAM(UIS_SET, UISF_HIDEFOCUS), 0);
	SetWindowSubclass(hDefaultCutCombo,
		[](HWND hCombo, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)->LRESULT
		{
			if (msg == WM_UPDATEUISTATE)
			{
				wParam &= ~MAKELONG(0, UISF_HIDEFOCUS | UISF_ACTIVE);
			}
			return DefSubclassProc(hCombo, msg, wParam, lParam);
		}, 0, 0);
	SendMessage(GetDlgItem(hDlg, IDOK), WM_UPDATEUISTATE, MAKEWPARAM(UIS_SET, UISF_HIDEFOCUS), 0);
	SetWindowSubclass(GetDlgItem(hDlg, IDOK), buttonSubclassProc, 0, 0);
	SendMessage(GetDlgItem(hDlg, IDCANCEL), WM_UPDATEUISTATE, MAKEWPARAM(UIS_SET, UISF_HIDEFOCUS), 0);
	SetWindowSubclass(GetDlgItem(hDlg, IDCANCEL), buttonSubclassProc, 0, 0);

	ComboBox_AddString(hDefaultCutCombo, _T("新建时输入"));
	ComboBox_AddString(hDefaultCutCombo, _T("使用默认值"));
	ComboBox_SetCurSel(hDefaultCutCombo, config.useDefaultCut);
	TCHAR str[6];
	_stprintf(str, _T("%04d"), (int)round((1 - config.defaultCut) * 10000));
	defaultCutEdit.setText(str);
	Button_SetCheck(hDefaultClosingCheck, config.defaultClosing);
	for (int i = 0; i < 4; i++)
	{
		_itot(config.fastAddedAmount[i], str, 10);
		fastAddedAmountEdit[i].setText(str);
	}
	_itot((int)round(config.defaultProbError * 100), str, 10);
	defaultProbErrorEdit.setText(str);

	return (INT_PTR)TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

INT_PTR ConfigDlg::dlgProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			{
				Config newConfig = { 0 };
				newConfig.useDefaultCut = ComboBox_GetCurSel(hDefaultCutCombo);
				TCHAR str[6];
				defaultCutEdit.getText(str, 6);
				for (int i = lstrlen(str) - 1; i >= 0; i--)
				{
					int c = str[i] - '0';
					newConfig.defaultCut = (newConfig.defaultCut + c) * 0.1;
				}
				newConfig.defaultCut = 1 - newConfig.defaultCut;
				newConfig.defaultClosing = Button_GetCheck(hDefaultClosingCheck);
				for (int i = 0; i < 4; i++)
				{
					fastAddedAmountEdit[i].getText(str, 6);
					int amount = _wtoi(str);
					if (amount == 0)
					{
						SetFocus(fastAddedAmountEdit[i].getHwnd());
						fastAddedAmountEdit[i].setSel(0, -1);
						return (INT_PTR)TRUE;
					}
					newConfig.fastAddedAmount[i] = amount;
				}
				defaultProbErrorEdit.getText(str, 6);
				newConfig.defaultProbError = _wtoi(str) / 100.0;
				if (memcmp(&newConfig, &config, sizeof(Config)) != 0)
				{
					ofstream file(CONFIG_FILE_NAME, ios::out | ios::binary);
					while (file.fail())
					{
						if (MessageBox(hDlg, _T("无法写入文件\"") CONFIG_FILE_NAME _T("\""), _T("bet设置"), MB_RETRYCANCEL | MB_ICONERROR) != IDRETRY)
						{
							return (INT_PTR)TRUE;
						}
						file.open(CONFIG_FILE_NAME, ios::out | ios::binary);
					}
					file.write((char*)&newConfig, sizeof(Config));
					file.close();
					config = newConfig;
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