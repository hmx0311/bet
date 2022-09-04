#include "framework.h"
#include "Edit.h"

#include "common.h"

WNDPROC defEditProc;

void setVCentered(HWND hEdit)
{
	RECT rect;
	GetClientRect(hEdit, &rect);
	LOGFONT logFont;
	GetObject(hFont, sizeof(LOGFONT), &logFont);
	rect.top += (rect.bottom - rect.top + logFont.lfHeight - 1.5f) / 2;
	SendMessage(hEdit, EM_SETRECTNP, 0, (LPARAM)&rect);
}

LRESULT CALLBACK editProc(HWND hEdit, UINT message, WPARAM wParam, LPARAM lParam)
{
	Edit* edit = (Edit*)GetWindowLongPtr(hEdit, GWLP_USERDATA);
	LRESULT result = (LRESULT)FALSE;
	if (edit != nullptr)
	{
		result = edit->wndProc(message, wParam, lParam);
	}
	if (message == WM_COMMAND)
	{
		switch (LOWORD(wParam))
		{
		case ID_CONFIRM:
		case ID_CANCEL:
			SendMessage(GetParent(hEdit), message, wParam, lParam);
			break;
		}
	}
	if (result)
	{
		return result;
	}
	return CallWindowProc(defEditProc, hEdit, message, wParam, lParam);
}

void Edit::attach(HWND hEdit)
{
	this->hEdit = hEdit;
	SetWindowLongPtr(hEdit, GWLP_USERDATA, (LONG_PTR)this);
	SetWindowLongPtr(hEdit, GWLP_WNDPROC, (LONG_PTR)editProc);
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

void Edit::setText(LPCWSTR str)
{
	SetWindowText(hEdit, str);
	SendMessage(hEdit, EM_SETSEL, lstrlen(str), -1);
}

void Edit::getText(LPWSTR str, int nMaxCount)
{
	GetWindowText(hEdit, str, nMaxCount);
}

void Edit::setSel(int start, int end)
{
	SendMessage(hEdit, EM_SETSEL, start, end);
}