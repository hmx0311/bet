// bet.cpp : 定义应用程序的入口点。
//

#include "framework.h"
#include "bet.h"

#include "BetDlg.h"
#include "common.h"

#include <fstream>

#pragma comment(lib,"comctl32.lib")
#pragma comment(lib,"UxTheme.lib")
#pragma comment(lib,"imm32.lib")

using namespace std;


void loadConfig(Config& cfg);

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
		return 0;
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

void loadConfig(Config& cfg)
{
	fstream file(CONFIG_FILE_NAME, ios::_Nocreate | ios::in | ios::binary);
	if (file.good())
	{
		if (file.read((char*)&cfg, sizeof(Config)).good() &&
			cfg.defCut > 0 && cfg.defCut <= 1 &&
			cfg.fastAddedAmount[0] > 0 && cfg.fastAddedAmount[0] < 100000 &&
			cfg.fastAddedAmount[1] > 0 && cfg.fastAddedAmount[1] < 100000 &&
			cfg.fastAddedAmount[2] > 0 && cfg.fastAddedAmount[2] < 100000 &&
			cfg.fastAddedAmount[3] > 0 && cfg.fastAddedAmount[3] < 100000 &&
			cfg.defProbError >= 0 && cfg.defProbError < 1)
		{
			file.close();
			return;
		}
		file.close();
	}
	cfg = DEFAULT_CONFIG;
	file.open(CONFIG_FILE_NAME, ios::out | ios::binary);
	if (file.good())
	{
		file.write((char*)&cfg, sizeof(Config));
		file.close();
	}
}