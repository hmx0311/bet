#include "framework.h"
#include "text.h"

#include <CommCtrl.h>
#include <windowsx.h>

LRESULT CALLBACK copyableTextSubclassProc(HWND hText, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
	switch (msg)
	{
	case WM_CONTEXTMENU:
		{
			HMENU hMenu = CreatePopupMenu();
			AppendMenu(hMenu, MF_ENABLED, 1, _T("¸´ÖÆ(&C)"));
			if (TrackPopupMenu(hMenu, TPM_NONOTIFY | TPM_RETURNCMD, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), 0, hText, nullptr) == 1)
			{
				int length = GetWindowTextLength(hText) + 1;
				HANDLE hStr = GlobalAlloc(GMEM_MOVEABLE, length * sizeof(TCHAR));
				GetWindowText(hText, (PWSTR)GlobalLock(hStr), length);
				GlobalUnlock(hStr);
				OpenClipboard(hText);
				EmptyClipboard();
				SetClipboardData(CF_UNICODETEXT, hStr);
				CloseClipboard();
				//GlobalFree(hStr);
			}
			DestroyMenu(hMenu);
			return 0;
		}
	}
	return DefSubclassProc(hText, msg, wParam, lParam);
}