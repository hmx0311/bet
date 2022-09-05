#include "framework.h"
#include "ConfigDlg.h"

#include <cmath>
#include <fstream>

using namespace std;

ConfigDlg::ConfigDlg()
{
	nIDTemplate = IDD_CONFIG_DIALOG;
}

INT_PTR ConfigDlg::initDlg(HWND hDlg)
{
	Dialog::initDlg(hDlg);
	loadConfig(oldConfig);

	cutEdit.attach(GetDlgItem(hDlg, IDC_CUT_EDIT));
	defaultClosingCheck.attach(GetDlgItem(hDlg, IDC_DEFAULT_CLOSING_CHECK), oldConfig.defaultClosing);
	for (int i = 0; i < 4; i++)
	{
		fastAddedAmountEdit[i].attach(GetDlgItem(hDlg, IDC_FAST_ADDED_AMOUNT_EDIT1 + i));
	}
	defaultProbErrorEdit.attach(GetDlgItem(hDlg, IDC_DEFAULT_PROBABILTY_ERROR_EDIT));


	SendMessage(GetDlgItem(hDlg,IDOK), WM_UPDATEUISTATE, MAKEWPARAM(UIS_SET, UISF_HIDEFOCUS), 0);
	SetWindowLongPtr(GetDlgItem(hDlg, IDOK), GWLP_WNDPROC, (LONG_PTR)buttonProc);
	SendMessage(GetDlgItem(hDlg, IDCANCEL), WM_UPDATEUISTATE, MAKEWPARAM(UIS_SET, UISF_HIDEFOCUS), 0);
	SetWindowLongPtr(GetDlgItem(hDlg, IDCANCEL), GWLP_WNDPROC, (LONG_PTR)buttonProc);

	cutEdit.setTextLimit(3);
	TCHAR str[6];
	swprintf(str, 6, _T("%03d"), (int)round((1 - oldConfig.cut) * 1000));
	cutEdit.setText(str);
	for (int i = 0; i < 4; i++)
	{
		fastAddedAmountEdit[i].setTextLimit(5);
		swprintf(str, 6, _T("%d"), oldConfig.fastAddedAmount[i]);
		fastAddedAmountEdit[i].setText(str);
	}
	defaultProbErrorEdit.setTextLimit(5);
	swprintf(str, 6, _T("%d"), (int)round(oldConfig.defaultProbError * 100));
	defaultProbErrorEdit.setText(str);

	return INT_PTR(TRUE);  // 除非将焦点设置到控件，否则返回 TRUE
}

INT_PTR ConfigDlg::dlgProc(UINT message, WPARAM wParam, LPARAM lParam)
{

	switch (message)
	{
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_DEFAULT_CLOSING_CHECK:
			defaultClosingCheck.setCheck(!defaultClosingCheck.getCheck());
			break;
		case IDOK:
			{
				Config newConfig = { 0,0,0 };
				TCHAR str[6];
				cutEdit.getText(str, 6);
				for (int i = lstrlen(str) - 1; i >= 0; i--)
				{
					int c = str[i] - '0';
					newConfig.cut = (newConfig.cut + c) * 0.1;
				}
				newConfig.cut = 1 - newConfig.cut;
				newConfig.defaultClosing = defaultClosingCheck.getCheck();
				for (int i = 0; i < 4; i++)
				{
					fastAddedAmountEdit[i].getText(str, 6);
					int amount = _wtoi(str);
					if (amount == 0)
					{
						SetFocus(fastAddedAmountEdit[i].getHwnd());
						fastAddedAmountEdit[i].setSel(0, -1);
						return INT_PTR(TRUE);
					}
					newConfig.fastAddedAmount[i] = amount;
				}
				defaultProbErrorEdit.getText(str, 6);
				newConfig.defaultProbError = _wtoi(str) / 100.0;
				if (memcmp(&newConfig, &oldConfig, sizeof(Config)) != 0)
				{
					ofstream file(CONFIG_FILE_NAME, ios::out | ios::binary);
					while (file.fail())
					{
						if (MessageBox(hDlg, _T("无法写入文件\"") CONFIG_FILE_NAME _T("\""), _T("bet设置"), MB_RETRYCANCEL | MB_ICONERROR) != IDRETRY)
						{
							return INT_PTR(TRUE);
						}
						file.open(CONFIG_FILE_NAME, ios::out | ios::binary);
					}
					file.write((char*)&newConfig, sizeof(Config));
					file.close();
					config.defaultProbError = newConfig.defaultProbError;
					config.defaultClosing = newConfig.defaultClosing;
				}
				MessageBox(hDlg, _T("设置已更改，部分设置将在重启后生效"), _T("bet设置"), MB_OK | MB_ICONINFORMATION);
			}
		case IDCANCEL:
			EndDialog(hDlg, 0);
			break;
		}
	}
	return INT_PTR(FALSE);
}