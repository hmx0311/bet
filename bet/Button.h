#pragma once
#include <uxtheme.h>
#include <Vsstyle.h>

extern HTHEME hButtonTheme;

LRESULT CALLBACK buttonSubclassProc(HWND hButton, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);

class Button
{
private:
	PUSHBUTTONSTATES lastState = PBS_NORMAL;
protected:
	HWND hButton;
	HICON hIcon = nullptr;
	int iconWidth;
	int iconHeight;
	HBRUSH hBkgBrush = GetSysColorBrush(COLOR_BTNFACE);
	bool isTracking = false;
public:
	void attach(HWND hButton);
	LRESULT wndProc(UINT msg, WPARAM wParam, LPARAM lParam);
	HWND getHwnd();
	void setText(PCTSTR str);
	void setIcon(HICON hIcon);
	void setBkgBrush(HBRUSH hBkgBrush);
private:
	void drawButton(HDC hDC, PUSHBUTTONSTATES states, RECT& rcItem);
};
