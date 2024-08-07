#include "framework.h"
#include "BetDlg.h"

#include "controls.h"
#include "common.h"
#include "tooltip.h"
#include "ConfigDlg.h"

#include <windowsx.h>
#include <CommCtrl.h>
#include <ctime>

#define TAB_WIDTH lroundf(142 * xScale)
int TAB_HEIGHT;
#define ADD_TAB_X (TAB_WIDTH + 5)
int ADD_TAB_Y;
#define MAX_TAB 4
#define TAB_NAME_EDIT_MARGIN_X lroundf(2 * xScale)
#define TAB_NAME_EDIT_MARGIN_Y lroundf(3.45f * yScale)
int TAB_NAME_EDIT_X;
int TAB_NAME_EDIT_Y;
#define TAB_NAME_EDIT_WIDTH (TAB_WIDTH - 2 * TAB_NAME_EDIT_MARGIN_X)
#define TAB_NAME_EDIT_HEIGHT (TAB_HEIGHT - 2 * TAB_NAME_EDIT_MARGIN_Y)

#define MAX_TAB_NAME_LEN 30

HICON BetDlg::hIcon;

BetDlg::BetDlg()
	:Dialog(IDD_BET_DIALOG) {}


INT_PTR BetDlg::initDlg(HWND hDlg)
{
	Dialog::initDlg(hDlg);
	SNDMSG(hDlg, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
	SNDMSG(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);

	hBetTab = GetDlgItem(hDlg, IDC_BET_TAB);
	SNDMSG(hBetTab, WM_UPDATEUISTATE, MAKEWPARAM(UIS_SET, UISF_HIDEFOCUS | UISF_HIDEACCEL), 0);
	SetWindowSubclass(hBetTab, [](HWND hTabCtrl, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR, DWORD_PTR)->LRESULT
		{
			LRESULT result = noFocusRectSubclassProc(hTabCtrl, msg, wParam, lParam, 0, 0);
			switch (msg)
			{
			case WM_PAINT:
				{
					HDC hDC = GetDCEx(hTabCtrl, nullptr, DCX_PARENTCLIP);
					int curSel = TabCtrl_GetCurSel(hTabCtrl);
					RECT rect = { 2 + curSel * TAB_WIDTH, TAB_HEIGHT, 2 + (curSel + 1) * TAB_WIDTH, TAB_HEIGHT + 2 };
					FillRect(hDC, &rect, GetSysColorBrush(COLOR_APPWORKSPACE));
					ReleaseDC(hTabCtrl, hDC);
				}
				break;
			}
			return result;
		}, 0, 0);
	settingsButton.attach(GetDlgItem(hDlg, IDC_SETTINGS_BUTTON));
	addTabButton.attach(GetDlgItem(hDlg, IDC_ADD_TAB_BUTTON));

	calcPos();

	hTabNameEdit = CreateWindowEx(WS_EX_STATICEDGE, WC_EDIT, _T(""),
		WS_CHILD | WS_VISIBLE | ES_CENTER | ES_MULTILINE,
		TAB_NAME_EDIT_X, TAB_NAME_EDIT_Y, TAB_NAME_EDIT_WIDTH, TAB_NAME_EDIT_HEIGHT,
		hDlg, (HMENU)IDC_TAB_NAME_EDIT, hInst, nullptr);
	SetWindowSubclass(hTabNameEdit,
		[](HWND hEdit, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR, DWORD_PTR)->LRESULT
		{
			switch (msg)
			{
			case WM_DPICHANGED_AFTERPARENT:
				setVCentered(hEdit);
				return 0;
			case WM_KEYDOWN:
				switch (wParam)
				{
				case VK_RETURN:
					ShowWindow(hEdit, SW_HIDE);
					break;
				case VK_ESCAPE:
					SetWindowText(hEdit, _T(""));
					ShowWindow(hEdit, SW_HIDE);
					return 0;
				}
				break;
			}
			return DefSubclassProc(hEdit, msg, wParam, lParam);
		}, 0, 0);

	hButtonTheme = OpenThemeData(hDlg, _T("Button"));

	settingsButton.setIcon((HICON)LoadImage(hInst, MAKEINTRESOURCE(IDI_SETTINGS), IMAGE_ICON, 0, 0, LR_SHARED));
	TCHAR settingsTipText[] = _T("����");
	createToolTip(settingsButton.getHwnd(), hDlg, settingsTipText);

	SetWindowPos(hTabNameEdit, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	SetWindowFont(hTabNameEdit, hFont, FALSE);
	setVCentered(hTabNameEdit);

	createTab();
	DRAGLISTMSG = RegisterWindowMessage(DRAGLISTMSGSTRING);
	return (INT_PTR)FALSE;
}

INT_PTR BetDlg::dlgProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_DPICHANGED:
		DeleteFont(hBoldFont);
		RECT rcTab, rcDlg;
		GetWindowRect(hBetTab, &rcTab);
		MapWindowRect(HWND_DESKTOP, hDlg, &rcTab);
		GetWindowRect(hDlg, &rcDlg);
		MapWindowRect(HWND_DESKTOP, hDlg, &rcDlg);
		SetWindowPos(hDlg, nullptr, 0, 0, rcTab.right - 2 * rcDlg.left, rcTab.bottom - rcDlg.top - rcDlg.left, SWP_NOMOVE | SWP_NOZORDER | SWP_NOREDRAW);
		calcPos();
		SetWindowPos(hTabNameEdit, nullptr, 0, 0, TAB_NAME_EDIT_WIDTH, TAB_NAME_EDIT_HEIGHT, SWP_NOMOVE | SWP_NOZORDER | SWP_NOREDRAW);
		DeleteObject(hIcon);
		hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_BET));
		SNDMSG(hDlg, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
		SNDMSG(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
		needErase = true;
		InvalidateRect(hDlg, nullptr, TRUE);
		break;
	case WM_THEMECHANGED:
		CloseThemeData(hButtonTheme);
		hButtonTheme = OpenThemeData(hDlg, _T("Button"));
		needErase = true;
		InvalidateRect(hDlg, nullptr, TRUE);
		return (INT_PTR)TRUE;
	case WM_ERASEBKGND:
		if (!needErase)
		{
			return (INT_PTR)TRUE;
		}
		needErase = false;
		break;
	case WM_MOVE:
	case WM_MOVING:
		needErase = true;
		break;
	case WM_PARENTNOTIFY:
		if (LOWORD(wParam) == WM_DESTROY)
		{
			createBetTabDlg(cutInputDlg.getCut());
			EnableWindow(hBetTab, TRUE);
			needErase = true;
		}
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_TAB_NAME_EDIT:
			if (HIWORD(wParam) == EN_KILLFOCUS)
			{
				TCHAR str[MAX_TAB_NAME_LEN];
				GetWindowText(hTabNameEdit, str, MAX_TAB_NAME_LEN);
				if (str[0] != '\0')
				{
					int tabId = TabCtrl_GetCurSel(hBetTab);
					TabCtrl_DeleteItem(hBetTab, tabId);
					TCITEM tcItem;
					tcItem.mask = TCIF_TEXT;
					tcItem.pszText = str;
					TabCtrl_InsertItem(hBetTab, tabId, &tcItem);
				}
				SetWindowText(hTabNameEdit, _T(""));
				ShowWindow(hTabNameEdit, SW_HIDE);
				return (INT_PTR)TRUE;
			}
			break;
		case IDC_ADD_TAB_BUTTON:
			ShowWindow(currentDlg->getHwnd(), SW_HIDE);
			createTab();
			return (INT_PTR)TRUE;
		case IDC_SETTINGS_BUTTON:
			ConfigDlg configDlg;
			configDlg.dialogBox(hDlg);
			return (INT_PTR)TRUE;
		}
		break;
	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->idFrom == IDC_BET_TAB)
		{
			switch (((LPNMHDR)lParam)->code)
			{
			case TCN_SELCHANGE:
				{
					ShowWindow(currentDlg->getHwnd(), SW_HIDE);
					currentDlg = betTabDlgs[TabCtrl_GetCurSel(hBetTab)];
					ShowWindow(currentDlg->getHwnd(), SW_SHOW);
					lastSel = -1;
					return (INT_PTR)TRUE;
				}
			case NM_CLICK:
				{
					static clock_t lastClickTime = INT_MIN;
					int curSel = TabCtrl_GetCurSel(hBetTab);
					if (lastSel == curSel)
					{
						clock_t clickTime = clock();
						if (clickTime - lastClickTime < GetDoubleClickTime())
						{
							TCITEM cItem;
							TCHAR str[MAX_TAB_NAME_LEN];
							cItem.mask = TCIF_TEXT;
							cItem.pszText = str;
							cItem.cchTextMax = MAX_TAB_NAME_LEN;
							TabCtrl_GetItem(hBetTab, lastSel, &cItem);
							SetWindowText(hTabNameEdit, str);
							Edit_SetSel(hTabNameEdit, 0, -1);
							SetWindowPos(hTabNameEdit, HWND_TOP, TAB_NAME_EDIT_X + lastSel * TAB_WIDTH, TAB_NAME_EDIT_Y, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);
							SetFocus(hTabNameEdit);
							lastSel = -1;
							lastClickTime = INT_MIN;
							return (INT_PTR)TRUE;
						}
						lastClickTime = clickTime;
					}
					else
					{
						lastClickTime = clock();
						lastSel = curSel;
					}
					return (INT_PTR)TRUE;
				}
			case NM_RCLICK:
				if (IsWindowVisible(hTabNameEdit))
				{
					(INT_PTR)TRUE;
				}
				POINT cursorPos;
				GetCursorPos(&cursorPos);
				int tabId = TabCtrl_GetCurSel(hBetTab);
				RECT rcSelTab;
				TabCtrl_GetItemRect(hBetTab, tabId, &rcSelTab);
				MapWindowRect(hBetTab, HWND_DESKTOP, &rcSelTab);
				if (PtInRect(&rcSelTab, cursorPos))
				{
					HMENU hMenu = CreatePopupMenu();
					AppendMenu(hMenu, (betTabDlgs.size() > 1 ? MF_ENABLED : MF_GRAYED), 1, _T("ɾ������(&D)"));
					if (TrackPopupMenu(hMenu, TPM_BOTTOMALIGN | TPM_NONOTIFY | TPM_RETURNCMD, cursorPos.x, cursorPos.y, 0, hDlg, nullptr) == 1)
					{
						TCITEM cItem;
						TCHAR tabName[MAX_TAB_NAME_LEN];
						cItem.mask = TCIF_TEXT;
						cItem.pszText = tabName;
						cItem.cchTextMax = MAX_TAB_NAME_LEN;
						TabCtrl_GetItem(hBetTab, tabId, &cItem);
						TCHAR str[50];
						_stprintf(str, _T("ȷ��Ҫɾ�����¡�%s����"), tabName);
						if (MessageBox(hDlg, str, _T("bet"), MB_YESNO | MB_ICONQUESTION) == IDYES)
						{
							SetWindowPos(addTabButton.getHwnd(), HWND_TOP, ADD_TAB_X + (betTabDlgs.size() - 2) * TAB_WIDTH, ADD_TAB_Y, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);
							betTabDlgs.erase(betTabDlgs.begin() + tabId);
							TabCtrl_DeleteItem(hBetTab, tabId);
							if (tabId == betTabDlgs.size())
							{
								tabId--;
							}
							DestroyWindow(currentDlg->getHwnd());
							delete currentDlg;
							currentDlg = betTabDlgs[tabId];
							ShowWindow(currentDlg->getHwnd(), SW_SHOW);
							SetFocus(currentDlg->getHwnd());
							TabCtrl_SetCurSel(hBetTab, tabId);
						}
					}
					DestroyMenu(hMenu);
				}
				return (INT_PTR)TRUE;
			}
		}
		break;
	case WM_CLOSE:
		ShowWindow(hDlg, SW_RESTORE);
		SetWindowPos(hDlg, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		SetWindowPos(hDlg, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		if (MessageBox(hDlg, _T("ȷ��Ҫ�˳���bet����"), _T("bet"), MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2) != IDYES)
		{
			return (INT_PTR)TRUE;
		}
		for (Dialog* betTab : betTabDlgs)
		{
			DestroyWindow(betTab->getHwnd());
		}
		CloseThemeData(hButtonTheme);
		PostQuitMessage(0);
		break;
	}
	return (INT_PTR)FALSE;
}

void BetDlg::createBetTabDlg(double cut)
{
	int tabId = betTabDlgs.size();
	if (tabId == MAX_TAB - 1)
	{
		ShowWindow(addTabButton.getHwnd(), FALSE);
	}
	else
	{
		SetWindowPos(addTabButton.getHwnd(), HWND_TOP, ADD_TAB_X + tabId * TAB_WIDTH, ADD_TAB_Y, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);
	}
	currentDlg = new BetTabDlg(cut);
	currentDlg->createDialog(hDlg);
	betTabDlgs.push_back(currentDlg);
}

void BetDlg::createTab()
{
	int tabId = betTabDlgs.size();
	if (config.useDefCut)
	{
		createBetTabDlg(config.defCut);
	}
	else
	{
		currentDlg = &cutInputDlg;
		cutInputDlg.createDialog(hDlg);
		InvalidateRect(cutInputDlg.getHwnd(), nullptr, TRUE);
		ShowWindow(addTabButton.getHwnd(), SW_HIDE);
		EnableWindow(hBetTab, FALSE);
	}
	TCHAR str[2];
	_itot(tabId + 1, str, 10);
	TCITEM tcItem;
	tcItem.mask = TCIF_TEXT;
	tcItem.pszText = str;
	TabCtrl_InsertItem(hBetTab, tabId, &tcItem);
	TabCtrl_SetCurSel(hBetTab, tabId);
	SetWindowPos(hTabNameEdit, HWND_TOP, TAB_NAME_EDIT_X + tabId * TAB_WIDTH, TAB_NAME_EDIT_Y, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);
	SetFocus(hTabNameEdit);
}

void BetDlg::calcPos() const
{
	RECT pos;
	GetWindowRect(hBetTab, &pos);
	MapWindowRect(HWND_DESKTOP, hDlg, &pos);
	xScale = pos.right / 641.0f;
	yScale = pos.bottom / 530.0f;
	hFont = GetWindowFont(hDlg);
	hBoldFont = createBoldFont(hFont);

	SetWindowFont(GetDlgItem(hDlg, IDC_BALANCE_MODE_CAPTION), hBoldFont, FALSE);
	SetWindowFont(GetDlgItem(hDlg, IDC_WIN_PROB_MODE_CAPTION), hBoldFont, FALSE);

	TAB_HEIGHT = pos.bottom - pos.top - 3;
	ADD_TAB_Y = pos.bottom - TAB_HEIGHT - 2;
	TAB_NAME_EDIT_X = TAB_NAME_EDIT_MARGIN_X + 2;
	TAB_NAME_EDIT_Y = pos.bottom - TAB_HEIGHT + TAB_NAME_EDIT_MARGIN_Y - 1;

	TabCtrl_SetItemSize(hBetTab, TAB_WIDTH, TAB_HEIGHT);
}