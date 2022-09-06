#include "framework.h"
#include "NumericEdit.h"

using namespace std;

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

void NumericEdit::setText(LPCWSTR str)
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