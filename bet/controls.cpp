#include "framework.h"
#include "controls.h"

#include <CommCtrl.h>

LRESULT CALLBACK noFocusRectSubclassProc(HWND hCtrl, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
	switch (msg)
	{
	case WM_UPDATEUISTATE:
		wParam &= ~MAKELONG(0, UISF_HIDEFOCUS);
		break;
	}
	return DefSubclassProc(hCtrl, msg, wParam, lParam);
}