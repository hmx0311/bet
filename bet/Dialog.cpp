#include "framework.h"
#include "Dialog.h"

#include "Button.h"
#include "ListBox.h"
#include "common.h"

INT_PTR CALLBACK dlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_INITDIALOG:
		return ((Dialog*)lParam)->initDlg(hDlg);
	case WM_DRAWITEM:
		{
			LPDRAWITEMSTRUCT lpDrawItemStruct = (LPDRAWITEMSTRUCT)lParam;
			HDC hDC = lpDrawItemStruct->hDC;
			if (BufferedPaintRenderAnimation(lpDrawItemStruct->hwndItem, hDC))
			{
				return (INT_PTR)TRUE;
			}
			switch (lpDrawItemStruct->CtlType)
			{
			case ODT_BUTTON:
				{
					Button* button = (Button*)GetWindowLongPtr(lpDrawItemStruct->hwndItem, GWLP_USERDATA);
					if (button != nullptr)
					{
						button->drawItem(hDC, lpDrawItemStruct->itemState, lpDrawItemStruct->rcItem);
					}
					return (INT_PTR)TRUE;
				}
			case ODT_LISTBOX:
				if (lpDrawItemStruct->itemID == -1)
				{
					return (INT_PTR)TRUE;
				}
				ListBox* listBox = (ListBox*)GetWindowLongPtr(lpDrawItemStruct->hwndItem, GWLP_USERDATA);
				if (listBox == nullptr)
				{
					return (INT_PTR)TRUE;
				}
				int width = lpDrawItemStruct->rcItem.right - lpDrawItemStruct->rcItem.left;
				int height = lpDrawItemStruct->rcItem.bottom - lpDrawItemStruct->rcItem.top;
				HBITMAP bmp = CreateCompatibleBitmap(hDC, width, height);
				HDC hDCMem = CreateCompatibleDC(hDC);
				SelectObject(hDCMem, bmp);
				SelectObject(hDCMem, hFont);

				listBox->drawItem(hDCMem, lpDrawItemStruct->itemID, lpDrawItemStruct->itemState, lpDrawItemStruct->itemData, lpDrawItemStruct->rcItem);

				BitBlt(hDC, lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top, width, height, hDCMem, 0, 0, SRCCOPY);
				DeleteDC(hDCMem);
				DeleteObject(bmp);
			}
			return (INT_PTR)TRUE;
		}
	case WM_MEASUREITEM:
		((LPMEASUREITEMSTRUCT)lParam)->itemHeight = listItemHeight;
		return (INT_PTR)TRUE;
	}
	Dialog* dlg = (Dialog*)GetWindowLongPtr(hDlg, GWLP_USERDATA);
	if (dlg == nullptr)
	{
		return INT_PTR(FALSE);
	}
	return dlg->dlgProc(msg, wParam, lParam);
}

INT_PTR Dialog::initDlg(HWND hDlg)
{
	this->hDlg = hDlg;
	SetWindowLongPtr(hDlg, GWLP_USERDATA, (LONG_PTR)this);
	return INT_PTR(TRUE);
}

void Dialog::createDialog(HWND hWndParent)
{
	if (hDlg == nullptr)
	{
		CreateDialogParam(hInst, MAKEINTRESOURCE(nIDTemplate), hWndParent, ::dlgProc, (LPARAM)this);
	}
}

INT_PTR Dialog::dialogBox(HWND hWndParent)
{
	return DialogBoxParam(hInst, MAKEINTRESOURCE(nIDTemplate), hWndParent, ::dlgProc, (LPARAM)this);
}

HWND Dialog::getHwnd()
{
	return hDlg;
}
