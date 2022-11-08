﻿// bet.cpp : 定义应用程序的入口点。
//

#include "framework.h"
#include "bet.h"

#include "BetDlg.h"
#include "common.h"

#pragma comment(lib,"comctl32.lib")
#pragma comment(lib,"UxTheme.lib")

#define MAX_LOADSTRING 100


// 全局变量:
HINSTANCE hInst;                                // 当前实例

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ PWSTR     pCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(pCmdLine);

	hInst = hInstance; // 将实例句柄存储在全局变量中

	// 执行应用程序初始化:
	loadConfig(config);
	BufferedPaintInit();

	BetDlg::hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_BET));
	BetTabDlg::hResetIcon = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(IDI_RESET), IMAGE_ICON, 0, 0, LR_SHARED);
	BetTabDlg::hClearIcon = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(IDI_CLEAR), IMAGE_ICON, 0, 0, LR_SHARED);
	BetTabDlg::hTickIcon = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(IDI_TICK), IMAGE_ICON, 0, 0, LR_SHARED);
	BetTabDlg::hCalculatorIcon = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(IDI_CALCULATOR), IMAGE_ICON, 0, 0, LR_SHARED);

	BetDlg betDlg;
	betDlg.createDialog();

	if (!betDlg.getHwnd())
	{
		return FALSE;
	}

	MSG msg;
	

	// 主消息循环:
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		if (!IsDialogMessage(betDlg.getHwnd(), &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	BufferedPaintUnInit();

	return (int)msg.wParam;
}