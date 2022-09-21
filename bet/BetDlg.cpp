#include "framework.h"
#include "BetDlg.h"

#include "common.h"
#include "tooltip.h"
#include "ConfigDlg.h"

#include <windowsx.h>
#include <CommCtrl.h>
#include <cmath>
#include <ctime>

#define TAB_WIDTH (int)roundf(142 * xScale)
int TAB_HEIGHT;
#define ADD_TAB_X (TAB_WIDTH+4)
int ADD_TAB_Y;
#define MAX_TAB 4
#define TAB_NAME_EDIT_MARGIN_X (int)roundf(2*xScale)
#define TAB_NAME_EDIT_MARGIN_Y (int)roundf(3.45f*yScale)
int TAB_NAME_EDIT_X;
int TAB_NAME_EDIT_Y;
#define TAB_NAME_EDIT_WIDTH (TAB_WIDTH - 2 * TAB_NAME_EDIT_MARGIN_X)
#define TAB_NAME_EDIT_HEIGHT (TAB_HEIGHT - 2 * TAB_NAME_EDIT_MARGIN_Y)


extern int X_MOVE;

HICON BetDlg::hIcon;

BetDlg::BetDlg()
{
	nIDTemplate = IDD_BET_DIALOG;
}


INT_PTR BetDlg::initDlg(HWND hDlg)
{
	Dialog::initDlg(hDlg);
	SendMessage(hDlg, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
	SendMessage(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);

	hBetTab = GetDlgItem(hDlg, IDC_BET_TAB);
	settingsButton.attach(GetDlgItem(hDlg, IDC_SETTINGS_BUTTON));
	addTabButton.attach(GetDlgItem(hDlg, IDC_ADD_TAB_BUTTON));

	calcPos();

	currentTab = new BetTabDlg;
	currentTab->createDialog(hDlg);
	betTabs.push_back(currentTab);
	ShowWindow(currentTab->getHwnd(), SW_SHOW);
	calcBetTabPos();

	INITCOMMONCONTROLSEX icex = { sizeof(icex),ICC_STANDARD_CLASSES };
	InitCommonControlsEx(&icex);
	hTabNameEdit = CreateWindowEx(WS_EX_STATICEDGE, WC_EDIT, _T(""),
		WS_CHILD | WS_VISIBLE | ES_CENTER | ES_MULTILINE,
		TAB_NAME_EDIT_X, TAB_NAME_EDIT_Y, TAB_NAME_EDIT_WIDTH, TAB_NAME_EDIT_HEIGHT,
		hDlg, (HMENU)IDC_TAB_NAME_EDIT, hInst, nullptr);

	SetWindowSubclass(hTabNameEdit, editSubclassProc, 0, 0);


	hButtonTheme = OpenThemeData(settingsButton.getHwnd(), _T("Button"));

	settingsButton.setIcon((HICON)LoadImage(hInst, MAKEINTRESOURCE(IDI_SETTINGS), IMAGE_ICON, 0, 0, LR_SHARED));

	TCHAR settingsTipText[] = _T("设置");
	createToolTip(settingsButton.getHwnd(), hDlg, settingsTipText);

	TCITEM tcItem;
	tcItem.mask = TCIF_TEXT;
	TCHAR tabName[] = _T("1");
	tcItem.pszText = tabName;
	TabCtrl_InsertItem(hBetTab, 0, &tcItem);

	SetWindowPos(addTabButton.getHwnd(), HWND_TOP, ADD_TAB_X, ADD_TAB_Y, 0, 0, SWP_NOSIZE);

	SetWindowPos(hTabNameEdit, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	SetWindowFont(hTabNameEdit, (WPARAM)hFont, FALSE);
	setVCentered(hTabNameEdit);
	SetFocus(hTabNameEdit);

	TCHAR str[10];
	_stprintf(str, _T("抽水:%.1f%%"), (1 - config.cut) * 100);
	SetDlgItemTextW(hDlg, IDC_CUT_TEXT, str);
	return (INT_PTR)FALSE;
}

INT_PTR BetDlg::dlgProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_DPICHANGED:
		RECT rcTab, rcDlg;
		GetWindowRect(hBetTab, &rcTab);
		MapWindowRect(HWND_DESKTOP, hDlg, &rcTab);
		GetWindowRect(hDlg, &rcDlg);
		MapWindowRect(HWND_DESKTOP, hDlg, &rcDlg);
		SetWindowPos(hDlg, nullptr, 0, 0, rcTab.right - 2 * rcDlg.left, rcTab.bottom - rcDlg.top - rcDlg.left, SWP_NOMOVE | SWP_NOZORDER | SWP_NOREDRAW);
		calcPos();
		calcBetTabPos();
		SetWindowPos(hTabNameEdit, nullptr, 0, 0, TAB_NAME_EDIT_WIDTH, TAB_NAME_EDIT_HEIGHT, SWP_NOMOVE | SWP_NOZORDER | SWP_NOREDRAW);
		DeleteObject(hIcon);
		hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_BET));
		SendMessage(hDlg, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
		SendMessage(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
		needErase = true;
		break;
	case WM_THEMECHANGED:
		CloseThemeData(hButtonTheme);
		hButtonTheme = OpenThemeData(settingsButton.getHwnd(), _T("Button"));
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
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case ID_CONFIRM:
			if (GetFocus() == hTabNameEdit)
			{
				SetFocus(currentTab->getHwnd());
			}
			return (INT_PTR)TRUE;
		case ID_CANCEL:
			if (GetFocus() == hTabNameEdit)
			{
				SetWindowText(hTabNameEdit, _T(""));
				SetFocus(currentTab->getHwnd());
			}
			return (INT_PTR)TRUE;
		case ID_DELETE:
			{
				int tabId = TabCtrl_GetCurSel(hBetTab);
				TCITEM cItem;
				TCHAR tabName[30];
				cItem.mask = TCIF_TEXT;
				cItem.pszText = tabName;
				cItem.cchTextMax = 30;
				TabCtrl_GetItem(hBetTab, tabId, &cItem);
				TCHAR str[50];
				_stprintf(str, _T("确定要删除竞猜“%s”吗？"), tabName);
				if (MessageBox(hDlg, str, _T("bet"), MB_YESNO | MB_ICONQUESTION) != IDYES)
				{
					return (INT_PTR)TRUE;
				}
				if (betTabs.size() == MAX_TAB)
				{
					ShowWindow(addTabButton.getHwnd(), SW_SHOW);
				}
				else
				{
					SetWindowPos(addTabButton.getHwnd(), HWND_TOP, ADD_TAB_X + (betTabs.size() - 2) * TAB_WIDTH, ADD_TAB_Y, 0, 0, SWP_NOSIZE);
				}
				betTabs.erase(betTabs.begin() + tabId);
				TabCtrl_DeleteItem(hBetTab, tabId);
				if (tabId == betTabs.size())
				{
					tabId--;
				}
				DestroyWindow(currentTab->getHwnd());
				delete currentTab;
				currentTab = betTabs[tabId];
				ShowWindow(currentTab->getHwnd(), SW_SHOW);
				SetFocus(currentTab->getHwnd());
				TabCtrl_SetCurSel(hBetTab, tabId);
				return (INT_PTR)TRUE;
			}
		case IDC_TAB_NAME_EDIT:
			if (HIWORD(wParam) == EN_KILLFOCUS)
			{
				TCHAR str[20];
				GetWindowText(hTabNameEdit, str, 20);
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
				ShowWindow(hTabNameEdit, false);
				return (INT_PTR)TRUE;
			}
			break;
		case IDC_ADD_TAB_BUTTON:
			{
				int tabId = betTabs.size();
				if (tabId == MAX_TAB - 1)
				{
					ShowWindow(addTabButton.getHwnd(), false);
				}
				else
				{
					SetWindowPos(addTabButton.getHwnd(), HWND_TOP, ADD_TAB_X + tabId * TAB_WIDTH, ADD_TAB_Y, 0, 0, SWP_NOSIZE);
				}
				ShowWindow(currentTab->getHwnd(), SW_HIDE);
				currentTab = new BetTabDlg;
				betTabs.push_back(currentTab);
				currentTab->createDialog(hDlg);
				ShowWindow(currentTab->getHwnd(), SW_SHOW);
				TCHAR str[2];
				_itot(tabId + 1, str, 10);
				TCITEM tcItem;
				tcItem.mask = TCIF_TEXT;
				tcItem.pszText = str;
				TabCtrl_InsertItem(hBetTab, tabId, &tcItem);
				TabCtrl_SetCurSel(hBetTab, tabId);
				SetWindowPos(hTabNameEdit, HWND_TOP, TAB_NAME_EDIT_X + tabId * TAB_WIDTH, TAB_NAME_EDIT_Y, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);
				SetFocus(hTabNameEdit);
				return (INT_PTR)TRUE;
			}
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
					ShowWindow(currentTab->getHwnd(), SW_HIDE);
					currentTab = betTabs[TabCtrl_GetCurSel(hBetTab)];
					ShowWindow(currentTab->getHwnd(), SW_SHOW);
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
							TCHAR str[30];
							cItem.mask = TCIF_TEXT;
							cItem.pszText = str;
							cItem.cchTextMax = 30;
							TabCtrl_GetItem(hBetTab, lastSel, &cItem);
							SetWindowText(hTabNameEdit, str);
							Edit_SetSel(hTabNameEdit, 0, -1);
							SetWindowPos(hTabNameEdit, HWND_TOP, TAB_NAME_EDIT_X + lastSel * TAB_WIDTH, TAB_NAME_EDIT_Y, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);
							SetFocus(hTabNameEdit);
							lastSel = -1;
							lastClickTime = -1000;
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
				RECT rcSelTab;
				TabCtrl_GetItemRect(hBetTab, TabCtrl_GetCurSel(hBetTab), &rcSelTab);
				ScreenToClient(hBetTab, &cursorPos);

				if (PtInRect(&rcSelTab, cursorPos))
				{
					ClientToScreen(hBetTab, &cursorPos);
					HMENU menu = CreatePopupMenu();
					AppendMenu(menu, (betTabs.size() > 1 ? MF_ENABLED : MF_GRAYED), ID_DELETE, _T("删除竞猜"));
					TrackPopupMenu(menu, TPM_BOTTOMALIGN, cursorPos.x, cursorPos.y, 0, hDlg, nullptr);
					DestroyMenu(menu);
				}
				(INT_PTR)TRUE;
			}
		}
		break;
	case WM_CLOSE:
		ShowWindow(hDlg, SW_RESTORE);
		SetWindowPos(hDlg, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		SetWindowPos(hDlg, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		if (MessageBox(hDlg, _T("确定要退出“bet”吗？"), _T("bet"), MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2) != IDYES)
		{
			return (INT_PTR)TRUE;
		}
		for (BetTabDlg* betTab : betTabs)
		{
			DestroyWindow(betTab->getHwnd());
		}
		CloseThemeData(hButtonTheme);
		PostQuitMessage(0);
		break;
	}
	return (INT_PTR)FALSE;
}

BetTabDlg* BetDlg::getCurrentTab()
{
	return currentTab;
}

void BetDlg::calcPos()
{
	RECT pos;
	GetWindowRect(hBetTab, &pos);
	MapWindowRect(HWND_DESKTOP, hDlg, &pos);
	xScale = pos.right / 676.0f;
	yScale = pos.bottom / 530.0f;
	hFont = GetWindowFont(hDlg);
	LOGFONT logFont;
	GetObject(hFont, sizeof(LOGFONT), &logFont);
	listItemHeight = abs(logFont.lfHeight);
	logFont.lfWeight = FW_BOLD;
	DeleteObject(hBoldFont);
	hBoldFont = CreateFontIndirect(&logFont);
	TAB_HEIGHT = pos.bottom - pos.top - 3;
	ADD_TAB_Y = pos.bottom - TAB_HEIGHT - 2;
	TAB_NAME_EDIT_X = TAB_NAME_EDIT_MARGIN_X + 2;
	TAB_NAME_EDIT_Y = pos.bottom - TAB_HEIGHT + TAB_NAME_EDIT_MARGIN_Y - 1;

	TabCtrl_SetItemSize(hBetTab, TAB_WIDTH, TAB_HEIGHT);
}

void BetDlg::calcBetTabPos()
{
	RECT rect;
	GetWindowRect(GetDlgItem(currentTab->getHwnd(), IDC_L_BET_LIST), &rect);
	GetWindowRect(GetDlgItem(currentTab->getHwnd(), IDC_MOVE_SPIN), &rect);
	X_MOVE = rect.left - rect.right;
}