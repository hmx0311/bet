#include "framework.h"
#include "BetDlg.h"

#include "common.h"
#include "Edit.h"
#include "ConfigDlg.h"

#include <windowsx.h>
#include <CommCtrl.h>
#include <cmath>
#include <ctime>

#define TAB_WIDTH (int)roundf(142 * xScale)
#define ADD_TAB_X (TAB_WIDTH+4)
#define MAX_TAB 4
#define TAB_NAME_EDIT_MARGIN_X (int)roundf(2*xScale)
#define TAB_NAME_EDIT_MARGIN_Y (int)roundf(3.45f*yScale)

int nameEditPosX;
int nameEditPosY;
int addTabPosY;

HICON BetDlg::hIcon;
HICON BetDlg::hSettingsIcon;

BetDlg::BetDlg()
{
	nIDTemplate = IDD_BET_DIALOG;
}


INT_PTR BetDlg::initDlg(HWND hDlg)
{
	Dialog::initDlg(hDlg);
	SendMessage(hDlg, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
	SendMessage(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);

	defButtonProc = (WNDPROC)GetWindowLongPtr(GetDlgItem(hDlg, IDC_SETTINGS_BUTTON), GWLP_WNDPROC);

	betTab = GetDlgItem(hDlg, IDC_BET_TAB);
	settingsButton.attach(GetDlgItem(hDlg, IDC_SETTINGS_BUTTON));
	addTabButton.attach(GetDlgItem(hDlg, IDC_ADD_TAB_BUTTON));

	RECT pos;
	GetWindowRect(betTab, &pos);
	MapWindowRect(HWND_DESKTOP, hDlg, &pos);
	xScale = pos.right / 676.0f;
	yScale = pos.bottom / 530.0f;
	hFont = (HFONT)SendMessage(hDlg, WM_GETFONT, 0, 0);
	LOGFONT logFont;
	GetObject(hFont, sizeof(LOGFONT), &logFont);
	listItemHeight = -logFont.lfHeight;
	int tabHeight = pos.bottom - pos.top - 3;
	addTabPosY = pos.bottom - tabHeight - 2;
	nameEditPosX = TAB_NAME_EDIT_MARGIN_X + 2;
	nameEditPosY = pos.bottom - tabHeight + TAB_NAME_EDIT_MARGIN_Y - 1;

	tabNameEdit = CreateWindowEx(WS_EX_STATICEDGE, _T("EDIT"), _T(""),
		WS_CHILD | WS_VISIBLE | ES_CENTER | ES_MULTILINE,
		nameEditPosX, nameEditPosY, TAB_WIDTH - 2 * TAB_NAME_EDIT_MARGIN_X, tabHeight - 2 * TAB_NAME_EDIT_MARGIN_Y,
		hDlg, (HMENU)IDC_TAB_NAME_EDIT, hInst, nullptr);

	defEditProc = (WNDPROC)SetWindowLongPtr(tabNameEdit, GWLP_WNDPROC, (LONG_PTR)editProc);

	hButtonTheme = OpenThemeData(settingsButton.getHwnd(), _T("Button"));

	settingsButton.setIcon(hSettingsIcon);

	currentTab = new BetTabDlg;
	currentTab->createDialog(hDlg);
	betTabs.push_back(currentTab);
	ShowWindow(currentTab->getHwnd(), SW_SHOW);

	SendMessage(betTab, TCM_SETITEMSIZE, 0, MAKELPARAM(TAB_WIDTH, tabHeight));
	TCITEM tcItem;
	tcItem.mask = TCIF_TEXT;
	TCHAR tabName[] = _T("1");
	tcItem.pszText = tabName;
	SendMessage(betTab, TCM_INSERTITEM, 0, (LPARAM)&tcItem);

	SetWindowPos(addTabButton.getHwnd(), nullptr, ADD_TAB_X, addTabPosY, 0, 0, SWP_NOSIZE);
	SendMessage(tabNameEdit, WM_SETFONT, (WPARAM)hFont, TRUE);
	SetWindowPos(tabNameEdit, nullptr, nameEditPosX, nameEditPosY, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);

	setVCentered(tabNameEdit);
	SetFocus(tabNameEdit);

	TCHAR str[10];
	swprintf(str, 10, _T("抽水:%.1f%%"), (1 - config.cut) * 100);
	SetDlgItemTextW(hDlg, IDC_CUT_TEXT, str);
	return (INT_PTR)FALSE;
}

INT_PTR BetDlg::dlgProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
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
			if (GetFocus() == tabNameEdit)
			{
				SetFocus(currentTab->getHwnd());
			}
			return (INT_PTR)TRUE;
		case ID_CANCEL:
			if (GetFocus() == tabNameEdit)
			{
				SetWindowText(tabNameEdit, _T(""));
				SetFocus(currentTab->getHwnd());
			}
			return (INT_PTR)TRUE;
		case ID_DELETE:
			{
				int tabId = SendMessage(betTab, TCM_GETCURSEL, 0, 0);
				TCITEM cItem;
				TCHAR tabName[30];
				cItem.mask = TCIF_TEXT;
				cItem.pszText = tabName;
				cItem.cchTextMax = 30;
				SendMessage(betTab, TCM_GETITEM, tabId, (LPARAM)&cItem);
				TCHAR str[50];
				swprintf(str, 50, _T("确定要删除竞猜“%s”吗？"), tabName);
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
					SetWindowPos(addTabButton.getHwnd(), nullptr, ADD_TAB_X + (betTabs.size() - 2) * TAB_WIDTH, addTabPosY, 0, 0, SWP_NOSIZE);
				}
				betTabs.erase(betTabs.begin() + tabId);
				SendMessage(betTab, TCM_DELETEITEM, tabId, 0);
				if (tabId == betTabs.size())
				{
					tabId--;
				}
				DestroyWindow(currentTab->getHwnd());
				delete currentTab;
				currentTab = betTabs[tabId];
				ShowWindow(currentTab->getHwnd(), SW_SHOW);
				SetFocus(currentTab->getHwnd());
				SendMessage(betTab, TCM_SETCURSEL, tabId, 0);
				return (INT_PTR)TRUE;
			}
		case IDC_TAB_NAME_EDIT:
			if (HIWORD(wParam) == EN_KILLFOCUS)
			{
				TCHAR str[20];
				GetWindowText(tabNameEdit, str, 20);
				if (str[0] != '\0')
				{
					int tabId = SendMessage(betTab, TCM_GETCURSEL, 0, 0);
					SendMessage(betTab, TCM_DELETEITEM, tabId, 0);
					TCITEM tcItem;
					tcItem.mask = TCIF_TEXT;
					tcItem.pszText = str;
					SendMessage(betTab, TCM_INSERTITEM, tabId, (LPARAM)&tcItem);
				}
				SetWindowText(tabNameEdit, _T(""));
				ShowWindow(tabNameEdit, false);
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
					SetWindowPos(addTabButton.getHwnd(), nullptr, ADD_TAB_X + tabId * TAB_WIDTH, addTabPosY, 0, 0, SWP_NOSIZE);
				}
				ShowWindow(currentTab->getHwnd(), SW_HIDE);
				currentTab = new BetTabDlg;
				betTabs.push_back(currentTab);
				currentTab->createDialog(hDlg);
				ShowWindow(currentTab->getHwnd(), SW_SHOW);
				TCHAR str[2];
				_itow(tabId + 1, str, 10);
				TCITEM tcItem;
				tcItem.mask = TCIF_TEXT;
				tcItem.pszText = str;
				SendMessage(betTab, TCM_INSERTITEM, tabId, (LPARAM)&tcItem);
				SendMessage(betTab, TCM_SETCURSEL, tabId, 0);
				SetWindowPos(tabNameEdit, nullptr, nameEditPosX + tabId * TAB_WIDTH, nameEditPosY, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);
				SetFocus(tabNameEdit);
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
					int tabId = SendMessage(betTab, TCM_GETCURSEL, 0, 0);
					ShowWindow(currentTab->getHwnd(), SW_HIDE);
					currentTab = betTabs[tabId];
					ShowWindow(currentTab->getHwnd(), SW_SHOW);
					lastSel = -1;
					return (INT_PTR)TRUE;
				}
			case NM_CLICK:
				{
					static clock_t lastClickTime = INT_MIN;
					int curSel = SendMessage(betTab, TCM_GETCURSEL, 0, 0);
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
							SendMessage(betTab, TCM_GETITEM, lastSel, (LPARAM)&cItem);
							SetWindowText(tabNameEdit, str);
							SendMessage(tabNameEdit, EM_SETSEL, 0, -1);
							SetWindowPos(tabNameEdit, nullptr, nameEditPosX + lastSel * TAB_WIDTH, nameEditPosY, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);
							SetFocus(tabNameEdit);
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
					SetFocus(currentTab->getHwnd());
					return (INT_PTR)TRUE;
				}
			case NM_RCLICK:
				if (IsWindowVisible(tabNameEdit))
				{
					(INT_PTR)TRUE;
				}
				POINT cursorPos;
				GetCursorPos(&cursorPos);
				RECT rcSelTab;
				SendMessage(betTab, TCM_GETITEMRECT, SendMessage(betTab, TCM_GETCURSEL, 0, 0), (LPARAM)&rcSelTab);
				ScreenToClient(betTab, &cursorPos);

				if (PtInRect(&rcSelTab, cursorPos))
				{
					ClientToScreen(betTab, &cursorPos);
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
	return INT_PTR(FALSE);
}

BetTabDlg* BetDlg::getCurrentTab()
{
	return currentTab;
}