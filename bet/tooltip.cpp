#include "framework.h"
#include "tooltip.h"

#include "common.h"

#include <CommCtrl.h>

HWND createToolTip(HWND hTool, HWND hDlg, PTSTR pszText)
{
	INITCOMMONCONTROLSEX icex = { sizeof(icex),ICC_TREEVIEW_CLASSES };
	InitCommonControlsEx(&icex);
	HWND hTip = CreateWindow(TOOLTIPS_CLASS, nullptr,
		WS_POPUP | TTS_ALWAYSTIP | TTS_NOPREFIX,
		CW_USEDEFAULT, CW_USEDEFAULT,
		CW_USEDEFAULT, CW_USEDEFAULT,
		hDlg, nullptr,
		hInst, nullptr);

	TOOLINFO toolInfo = { 0 };
	toolInfo.cbSize = sizeof(toolInfo);
	toolInfo.hwnd = hDlg;
	toolInfo.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
	toolInfo.uId = (UINT_PTR)hTool;
	toolInfo.lpszText = pszText;
	SendMessage(hTip, TTM_ADDTOOL, 0, (LPARAM)&toolInfo);
	return hTip;
}

void setToolTipText(HWND hTip, HWND hTool, HWND hDlg, PTSTR pszText)
{
	TOOLINFO toolInfo = { 0 };
	toolInfo.cbSize = sizeof(toolInfo);
	toolInfo.hwnd = hDlg;
	toolInfo.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
	toolInfo.uId = (UINT_PTR)hTool;
	toolInfo.lpszText = pszText;
	SendMessage(hTip, TTM_UPDATETIPTEXT, 0, (LPARAM)&toolInfo);
}
