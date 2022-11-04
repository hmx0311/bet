#include "framework.h"
#include "edit.h"

#include "common.h"
#include "bet.h"

#include <windowsx.h>

#define IDC_CLEAR_BUTTON 1000

using namespace std;

void setVCentered(HWND hEdit)
{
	RECT rcVCentered;
	GetClientRect(hEdit, &rcVCentered);
	LOGFONT logFont;
	GetObject(hFont, sizeof(LOGFONT), &logFont);
	rcVCentered.top = 0.5f * (rcVCentered.bottom - rcVCentered.top - (abs(logFont.lfHeight) + 1.5f));
	Edit_SetRectNoPaint(hEdit, &rcVCentered);
}

LRESULT CALLBACK editSubclassProc(HWND hEdit, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
	switch (msg)
	{
	case WM_CONTEXTMENU:
		{
			SetFocus(hEdit);
			RECT rect;
			GetWindowRect(hEdit, &rect);
			POINT pos = { lParam == -1 ? (rect.left + rect.right) / 2 : GET_X_LPARAM(lParam),lParam == -1 ? (rect.top + rect.bottom) / 2 : GET_Y_LPARAM(lParam) };
			HMENU hMenu = CreatePopupMenu();
			DWORD sel = Edit_GetSel(hEdit);
			AppendMenu(hMenu, Edit_CanUndo(hEdit) ? MF_ENABLED : MF_GRAYED, 1, _T("³·Ïú(&U)"));
			AppendMenu(hMenu, MF_SEPARATOR, 0, nullptr);
			AppendMenu(hMenu, HIWORD(sel) == LOWORD(sel) ? MF_GRAYED : MF_ENABLED, 2, _T("¼ôÇÐ(&T)"));
			AppendMenu(hMenu, HIWORD(sel) == LOWORD(sel) ? MF_GRAYED : MF_ENABLED, 3, _T("¸´ÖÆ(&C)"));
			AppendMenu(hMenu, IsClipboardFormatAvailable(CF_TEXT) ? MF_ENABLED : MF_GRAYED, 4, _T("Õ³Ìù(&P)"));
			AppendMenu(hMenu, HIWORD(sel) == LOWORD(sel) ? MF_GRAYED : MF_ENABLED, 5, _T("É¾³ý(&D)"));
			AppendMenu(hMenu, MF_SEPARATOR, 0, nullptr);
			AppendMenu(hMenu, LOWORD(sel) == 0 && HIWORD(sel) == GetWindowTextLength(hEdit) ? MF_GRAYED : MF_ENABLED, 6, _T("È«Ñ¡(&A)"));
			switch (TrackPopupMenu(hMenu, TPM_NONOTIFY | TPM_RETURNCMD, pos.x, pos.y, 0, hEdit, nullptr))
			{
			case 1:
				Edit_Undo(hEdit);
				break;
			case 2:
				SendMessage(hEdit, WM_CUT, 0, 0);
				break;
			case 3:
				SendMessage(hEdit, WM_COPY, 0, 0);
				break;
			case 4:
				SendMessage(hEdit, WM_PASTE, 0, 0);
				break;
			case 5:
				Edit_ReplaceSel(hEdit, _T(""));
				break;
			case 6:
				Edit_SetSel(hEdit, 0, INT_MAX);
				break;
			}
			DestroyMenu(hMenu);
			return 0;
		}
	}
	Edit* edit = (Edit*)GetWindowLongPtr(hEdit, GWLP_USERDATA);
	LRESULT result = edit == nullptr ? DefSubclassProc(hEdit, msg, wParam, lParam) : edit->wndProc(msg, wParam, lParam);
	switch (msg)
	{
	case WM_GETDLGCODE:
		return result & ~DLGC_HASSETSEL;
	}
	if (result)
	{
		return result;
	}
	switch (msg)
	{
	case WM_DPICHANGED_AFTERPARENT:
		setVCentered(hEdit);
		return 0;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case ID_CONFIRM:
			SendMessage(GetParent(hEdit), msg, wParam, lParam);
			break;
		}
		break;
	}
	return (LRESULT)FALSE;
}


//abstract class Edit

void Edit::attach(HWND hEdit)
{
	this->hEdit = hEdit;
	SetWindowLongPtr(hEdit, GWLP_USERDATA, (LONG_PTR)this);
	SetWindowSubclass(hEdit, editSubclassProc, 0, 0);
	setVCentered(hEdit);
}

HWND Edit::getHwnd()
{
	return hEdit;
}

void Edit::setText(PCTSTR str)
{
	SetWindowText(hEdit, str);
	Edit_SetSel(hEdit, lstrlen(str), -1);
}

void Edit::getText(PTSTR str, int nMaxCount)
{
	GetWindowText(hEdit, str, nMaxCount);
}

void Edit::setSel(int start, int end)
{
	Edit_SetSel(hEdit, start, end);
}


//class NumericEdit

LRESULT NumericEdit::wndProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case EM_CANUNDO:
		{
			TCHAR temp[20];
			getText(temp, 20);
			return !curUndo.empty() && (lstrcmp(temp, curUndo.c_str()) != 0 || !lastUndo.empty());
		}
	case EM_UNDO:
		{
			TCHAR temp[20];
			getText(temp, 20);
			if (curUndo.empty())
			{
				curUndo = temp;
				return (LRESULT)TRUE;
			}
			if (lstrcmp(temp, curUndo.c_str()) == 0)
			{
				if (lastUndo.empty())
				{
					return (LRESULT)TRUE;
				}
				lastUndo.swap(curUndo);
			}
			else if (temp[0] != '\0')
			{
				lastUndo = temp;
			}
			Edit::setText(curUndo.c_str());
			return (LRESULT)TRUE;
		}
	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_ESCAPE:
			Edit::setText(curUndo.c_str());
			break;
		}
		break;
	case WM_PASTE:
		{
			wstring str;
			OpenClipboard(hEdit);
			char* text = (char*)GetClipboardData(CF_TEXT);
			if (text == nullptr)
			{
				CloseClipboard();
				return 0;
			}
			for (; *text != '\0'; text++)
			{
				switch (*text)
				{
				case '\t':
				case '\r':
				case '\n':
				case ' ':
				case ',':
					break;
				default:
					if (*text < 0)
					{
						if (*(WCHAR*)text == '¡¡')
						{
							text++;
							break;
						}
						str.clear();
					}
					if (isdigit(*text))
					{
						str.push_back(*text);
					}
					else
					{
						str.clear();
					}
				}
			}
			CloseClipboard();
			Edit_ReplaceSel(hEdit, str.c_str());
			return 0;
		}
	case WM_KILLFOCUS:
		updateStr();
		break;
	case WM_SHOWWINDOW:
		lastUndo = _T("");
		curUndo = _T("");
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case ID_CONFIRM:
			updateStr();
			break;
		}
		break;
	}
	return DefSubclassProc(hEdit, msg, wParam, lParam);
}

void NumericEdit::setText(PCTSTR str)
{
	updateStr();
	if (str[0] != '\0' && str != curUndo)
	{
		lastUndo = curUndo;
		curUndo = str;
	}
	Edit::setText(str);
}

void NumericEdit::updateStr()
{
	TCHAR temp[10];
	getText(temp, 10);
	if (temp[0] == '\0' || temp == curUndo)
	{
		return;
	}
	lastUndo = curUndo;
	curUndo = temp;
}


//class AmountEdit

void AmountEdit::attach(HWND hEdit, HWND hBankerOddsEdit, HWND hBetOddsEdit, HWND hBankerSelector)
{
	Edit::attach(hEdit);
	this->hBankerOddsEdit = hBankerOddsEdit;
	this->hBetOddsEdit = hBetOddsEdit;
	this->hBankerSelector = hBankerSelector;

	INITCOMMONCONTROLSEX icex = { sizeof(icex),ICC_STANDARD_CLASSES };
	InitCommonControlsEx(&icex);
	HWND hClearButton = CreateWindow(_T("Button"), nullptr,
		WS_CHILD | WS_VISIBLE | BS_FLAT,
		0, 0, 0, 0,
		hEdit, (HMENU)IDC_CLEAR_BUTTON, hInst, nullptr);
	clearButton.attach(hClearButton);
	clearButton.setIcon((HICON)LoadImage(hInst, MAKEINTRESOURCE(IDI_CLEAR), IMAGE_ICON, 0, 0, LR_SHARED));
	clearButton.setBkgBrush(GetSysColorBrush(COLOR_WINDOW));
	setClearButtonPos();
}

LRESULT AmountEdit::wndProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_DPICHANGED_AFTERPARENT:
		setClearButtonPos();
		break;
	case WM_KEYDOWN:
	case WM_KEYUP:
		switch (LOWORD(wParam))
		{
		case VK_UP:
		case VK_DOWN:
			SendMessage(Button_GetCheck(hBankerSelector) ? hBankerOddsEdit : hBetOddsEdit, msg, wParam, lParam);
			return 0;
		}
		break;
	case WM_KILLFOCUS:
		SendMessage(Button_GetCheck(hBankerSelector) ? hBankerOddsEdit : hBetOddsEdit, WM_KEYUP, VK_UP, MAKELONG(1, KF_UP | KF_REPEAT | KF_EXTENDED));
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_CLEAR_BUTTON:
			setText(_T(""));
			SetFocus(hEdit);
			return 0;
		}
		break;
	}
	return NumericEdit::wndProc(msg, wParam, lParam);
}

void AmountEdit::setClearButtonPos()
{
	RECT rcEdit;
	GetWindowRect(hEdit, &rcEdit);
	MapWindowRect(HWND_DESKTOP, hEdit, &rcEdit);
	int buttonPadding = (rcEdit.bottom - rcEdit.top) / 6;
	SetWindowPos(clearButton.getHwnd(), nullptr,
		rcEdit.right - (rcEdit.bottom - rcEdit.top - buttonPadding), rcEdit.top + buttonPadding, rcEdit.bottom - rcEdit.top - 2 * buttonPadding, rcEdit.bottom - rcEdit.top - 2 * buttonPadding,
		SWP_NOZORDER);
}


//class OddsEdit

LRESULT OddsEdit::wndProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case EM_SETSEL:
		if (wParam == 0 && lParam == -1)
		{
			return 0;
		}
		break;
	case WM_MOUSEWHEEL:
		SetFocus(hEdit);
		break;
	case WM_KILLFOCUS:
		SendMessage(hEdit, WM_KEYUP, VK_UP, MAKELONG(1, KF_UP | KF_REPEAT | KF_EXTENDED));
		updateOdds();
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case ID_CONFIRM:
			updateOdds();
			break;
		}
		break;
	}
	return DefSubclassProc(hEdit, msg, wParam, lParam);
}

double OddsEdit::getOdds()
{
	return odds;
}

void OddsEdit::spinDelta(int iDelta)
{
	if (iDelta < 0)
	{
		oddsUp(-0.1 * iDelta);
	}
	else
	{
		oddsDown(0.1 * iDelta);
	}
}

void OddsEdit::oddsUp(double up)
{
	updateOdds();
	if (odds < 9.85)
	{
		odds += up;
		if (odds > 9.9)
		{
			odds = 9.9;
		}
		TCHAR str[4];
		_stprintf(str, _T("%0.1f"), odds);
		setText(str);
	}
}

void OddsEdit::oddsDown(double down)
{
	updateOdds();
	if (odds > 0.15)
	{
		odds -= down;
		if (odds < 0.1)
		{
			odds = 0.1;
		}
		TCHAR str[4];
		_stprintf(str, _T("%0.1f"), odds);
		setText(str);
	}
}

void OddsEdit::updateOdds()
{
	TCHAR str[4];
	getText(str, 4);
	int length = lstrlen(str);
	TCHAR c;
	if (length > 0 && (c = str[0]) >= '0' && c <= '9')
	{
		double result = c - '0';
		if (length > 1)
		{
			c = str[1];
			if (c == '.')
			{
				if (length == 2)
				{
					if (result != 0)
					{
						odds = result;
					}
				}
				else if ((c = str[2]) >= '0' && c <= '9')
				{
					result += (c - '0') * 0.1;
					if (result != 0)
					{
						odds = result;
						return;
					}
				}
			}
			else if (length == 2 && c >= '0' && c <= '9')
			{
				result += (c - '0') * 0.1;
				if (result != 0)
				{
					odds = result;
				}
			}
		}
		else if (result != 0)
		{
			odds = result;
		}
	}
	else if (length == 2 && c == '.' && (c = str[1]) >= '1' && c <= '9')
	{
		odds = (c - '0') * 0.1;
	}
	_stprintf(str, _T("%0.1f"), odds);
	setText(str);
}