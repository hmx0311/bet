#include "framework.h"
#include "Edit.h"

#include "common.h"
#include "bet.h"

#include <CommCtrl.h>
#include <windowsx.h>

using namespace std;

void setVCentered(HWND hEdit)
{
	RECT rcEdit;
	GetWindowRect(hEdit, &rcEdit);
	MapWindowRect(HWND_DESKTOP, hEdit, &rcEdit);
	RECT rcClient;
	GetClientRect(hEdit, &rcClient);
	LOGFONT logFont;
	GetObject(hFont, sizeof(LOGFONT), &logFont);
	rcClient.top = rcEdit.top + 0.5f * (rcEdit.bottom - rcEdit.top - (abs(logFont.lfHeight) + 1.5f));
	SendMessage(hEdit, EM_SETRECTNP, 0, (LPARAM)&rcClient);
}

LRESULT CALLBACK editSubclassProc(HWND hEdit, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
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
		return (LRESULT)TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case ID_CONFIRM:
		case ID_CANCEL:
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
	SendMessage(hEdit, EM_SETSEL, lstrlen(str), -1);
}

void Edit::getText(PTSTR str, int nMaxCount)
{
	GetWindowText(hEdit, str, nMaxCount);
}

void Edit::setSel(int start, int end)
{
	SendMessage(hEdit, EM_SETSEL, start, end);
}


//class NumericEdit

LRESULT NumericEdit::wndProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case EM_UNDO:
		{
			TCHAR temp[10];
			getText(temp, 10);
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
	case WM_PASTE:
		{
			OpenClipboard(hEdit);
			wstring str;
			char* text = (char*)GetClipboardData(CF_TEXT);
			if (text == nullptr)
			{
				return (INT_PTR)TRUE;
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
			SendMessage(hEdit, EM_REPLACESEL, (WPARAM)0, (LPARAM)str.c_str());
			return (LRESULT)TRUE;
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
		case ID_CANCEL:
			Edit::setText(curUndo.c_str());
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
	this->hEdit = hEdit;
	this->hBankerOddsEdit = hBankerOddsEdit;
	this->hBetOddsEdit = hBetOddsEdit;
	this->hBankerSelector = hBankerSelector;
	SetWindowLongPtr(hEdit, GWLP_USERDATA, (LONG_PTR)this);
	SetWindowSubclass(hEdit, editSubclassProc, 0, 0);
	initRect();
}

LRESULT AmountEdit::wndProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_DPICHANGED_AFTERPARENT:
		initRect();
		return (LRESULT)TRUE;
	case WM_KEYDOWN:
	case WM_KEYUP:
		switch (LOWORD(wParam))
		{
		case VK_UP:
		case VK_DOWN:
			SendMessage(SendMessage(hBankerSelector, BM_GETCHECK, 0, 0) ? hBankerOddsEdit : hBetOddsEdit, msg, wParam, lParam);
			return (LRESULT)TRUE;
		}
		break;
	case WM_KILLFOCUS:
		SendMessage(SendMessage(hBankerSelector, BM_GETCHECK, 0, 0) ? hBankerOddsEdit : hBetOddsEdit, WM_KEYUP, VK_UP, MAKELONG(1, KF_UP | KF_REPEAT | KF_EXTENDED));
		break;
	}
	return NumericEdit::wndProc(msg, wParam, lParam);
}

void AmountEdit::initRect()
{
	RECT rcEdit;
	GetWindowRect(hEdit, &rcEdit);
	MapWindowRect(HWND_DESKTOP, hEdit, &rcEdit);
	RECT rcClient;
	GetClientRect(hEdit, &rcClient);
	LOGFONT logFont;
	GetObject(hFont, sizeof(LOGFONT), &logFont);
	rcClient.right -= rcClient.bottom - rcClient.top;
	rcClient.top = rcEdit.top + 0.5f * (rcEdit.bottom - rcEdit.top - (abs(logFont.lfHeight) + 1.5f));
	SendMessage(hEdit, EM_SETRECTNP, 0, (LPARAM)&rcClient);
}


//class OddsEdit

LRESULT OddsEdit::wndProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case EM_SETSEL:
		if (wParam == 0 && isSpinSel)
		{
			isSpinSel = false;
			return (LRESULT)TRUE;
		}
		break;
	case WM_KILLFOCUS:
		SendMessage(hEdit, WM_KEYUP, VK_UP, MAKELONG(1, KF_UP | KF_REPEAT | KF_EXTENDED));
		updateOdds();
		break;
	case WM_MOUSEWHEEL:
		if ((short)(HIWORD(wParam)) < 0)
		{
			oddsDown();
		}
		else
		{
			oddsUp();
		}
		SetFocus(hEdit);
		return (LRESULT)TRUE;
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
	isSpinSel = true;
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