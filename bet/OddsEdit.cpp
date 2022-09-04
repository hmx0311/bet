#include "framework.h"
#include "OddsEdit.h"

LRESULT OddsEdit::wndProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
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
	swprintf(str, 4, _T("%0.1f"), odds);
	setText(str);
}

void OddsEdit::oddsUp()
{
	if (odds < 9.85)
	{
		odds += 0.1;
		TCHAR str[4];
		swprintf(str, 4, _T("%0.1f"), odds);
		setText(str);
	}
}

void OddsEdit::oddsDown()
{
	if (odds > 0.15)
	{
		odds -= 0.1;
		TCHAR str[4];
		swprintf(str, 4, _T("%0.1f"), odds);
		setText(str);
	}
}
