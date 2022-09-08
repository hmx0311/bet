#include "framework.h"
#include "Edit.h"

#include "common.h"

#include <CommCtrl.h>

using namespace std;

void setVCentered(HWND hEdit)
{
	RECT rect;
	GetClientRect(hEdit, &rect);
	LOGFONT logFont;
	GetObject(hFont, sizeof(LOGFONT), &logFont);
	rect.top += (rect.bottom - rect.top + logFont.lfHeight - 1.5f) / 2;
	SendMessage(hEdit, EM_SETRECTNP, 0, (LPARAM)&rect);
}

LRESULT CALLBACK editSubclassProc(HWND hEdit, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
	Edit* edit = (Edit*)GetWindowLongPtr(hEdit, GWLP_USERDATA);
	LRESULT result = (LRESULT)FALSE;
	if (edit != nullptr)
	{
		result = edit->wndProc(msg, wParam, lParam);
	}
	if (msg == WM_COMMAND)
	{
		switch (LOWORD(wParam))
		{
		case ID_CONFIRM:
		case ID_CANCEL:
			SendMessage(GetParent(hEdit), msg, wParam, lParam);
			break;
		}
	}
	if (result)
	{
		return result;
	}
	return DefSubclassProc(hEdit, msg, wParam, lParam);
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

void Edit::getRect(RECT* rect)
{
	SendMessage(hEdit, EM_GETRECT, 0, (LPARAM)rect);
}

void Edit::setRectNP(RECT* rect)
{
	SendMessage(hEdit, EM_SETRECTNP, 0, (LPARAM)rect);
}

void Edit::setTextLimit(int limit)
{
	SendMessage(hEdit, EM_SETLIMITTEXT, limit, 0);
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
	case WM_KEYDOWN:
		switch (LOWORD(wParam))
		{
		case VK_UP:
		case VK_DOWN:
			SendMessage(GetParent(hEdit), msg, wParam, lParam);
			return (LRESULT)TRUE;
		}
		break;
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
			SendMessage(GetParent(hEdit), WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(hEdit), EN_CHANGE), 0);
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
			return (LRESULT)TRUE;
		case ID_CANCEL:
			Edit::setText(curUndo.c_str());
			return (LRESULT)TRUE;
		}
		break;
	}
	return LRESULT(FALSE);
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


//class OddsEdit

LRESULT OddsEdit::wndProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_KILLFOCUS:
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
	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_UP:
			oddsUp();
			return (LRESULT)TRUE;
		case  VK_DOWN:
			oddsDown();
			return (LRESULT)TRUE;
		}
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case ID_CONFIRM:
			updateOdds();
			return (LRESULT)TRUE;
		}
		break;
	}
	return (LRESULT)FALSE;
}

double OddsEdit::getOdds()
{
	return odds;
}

void OddsEdit::oddsUp(double up)
{
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