#include "framework.h"
#include "Dialog.h"

#include "listBox.h"
#include "common.h"

INT_PTR CALLBACK dlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_INITDIALOG:
		return ((Dialog*)lParam)->initDlg(hDlg);
	case WM_DRAWITEM:
		{
			PDRAWITEMSTRUCT pDrawItemStruct = (PDRAWITEMSTRUCT)lParam;
			switch (pDrawItemStruct->CtlType)
			{
			case ODT_LISTBOX:
				if (pDrawItemStruct->itemID == -1)
				{
					return (INT_PTR)TRUE;
				}
				((ListBox*)GetWindowLongPtr(pDrawItemStruct->hwndItem, GWLP_USERDATA))->
					drawItem(pDrawItemStruct->hDC, pDrawItemStruct->itemID, pDrawItemStruct->itemState, pDrawItemStruct->itemData, pDrawItemStruct->rcItem);
			}
			return (INT_PTR)TRUE;
		}
	case WM_MEASUREITEM:
		((PMEASUREITEMSTRUCT)lParam)->itemHeight = listItemHeight;
		return (INT_PTR)TRUE;
	}
	Dialog* dlg = (Dialog*)GetWindowLongPtr(hDlg, GWLP_USERDATA);
	return dlg == nullptr ? (INT_PTR)FALSE : dlg->dlgProc(msg, wParam, lParam);
}

Dialog::Dialog(UINT nIDTemplate)
	:nIDTemplate(nIDTemplate) {}

INT_PTR Dialog::initDlg(HWND hDlg)
{
	this->hDlg = hDlg;
	SetWindowLongPtr(hDlg, GWLP_USERDATA, (LONG_PTR)this);
	return (INT_PTR)TRUE;
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