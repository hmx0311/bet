// bet.cpp : 定义应用程序的入口点。
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

// 此代码模块中包含的函数的前向声明:
//INT_PTR CALLBACK    betDlgProc(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ PTSTR     pCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(pCmdLine);

	hInst = hInstance; // 将实例句柄存储在全局变量中

	// 执行应用程序初始化:
	loadConfig(config);

	BufferedPaintInit();

	BetDlg::hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_BET));
	BetDlg::hSettingsIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SETTINGS));
	BetTabDlg::hResetIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_RESET));
	BetTabDlg::hClearIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_CLEAR));
	BetTabDlg::hTickIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TICK));
	BetTabDlg::hCalculatorIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_CALCULATOR));

	HACCEL hAccel = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDR_ACCELERATOR));

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
		if (!TranslateAccelerator(msg.hwnd, hAccel, &msg) &&
			!IsDialogMessage(betDlg.getCurrentTab()->getHwnd(), &msg) &&
			!IsDialogMessage(betDlg.getHwnd(), &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	BufferedPaintUnInit();

	DeleteObject(BetDlg::hIcon);
	DeleteObject(BetDlg::hSettingsIcon);
	DeleteObject(BetTabDlg::hResetIcon);
	DeleteObject(BetTabDlg::hClearIcon);
	DeleteObject(BetTabDlg::hTickIcon);
	DeleteObject(BetTabDlg::hCalculatorIcon);

	return (int)msg.wParam;
}