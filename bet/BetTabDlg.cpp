#include "framework.h"
#include "BetTabDlg.h"

#include "common.h"

#include "tooltip.h"

#include <CommCtrl.h>
#include <Vsstyle.h>
#include <uxtheme.h>
#include <shobjidl_core.h>
#include <shellapi.h>
#include <windowsx.h>

#define BPC_CONNECTED (WM_APP)
#define BPC_DISCONNECT (WM_APP+1)
#define BPC_PROBABILITY (WM_APP+2)

#define LEFT_SIDE 0
#define RIGHT_SIDE 1

extern int X_MOVE;

HICON BetTabDlg::hResetIcon;
HICON BetTabDlg::hClearIcon;
HICON BetTabDlg::hTickIcon;
HICON BetTabDlg::hCalculatorIcon;

BetTabDlg::BetTabDlg()
{
	nIDTemplate = IDD_BET_TAB_DIALOG;
}

INT_PTR BetTabDlg::initDlg(HWND hDlg)
{
	Dialog::initDlg(hDlg);

	if (X_MOVE == 0)
	{
		RECT rect;
		GetWindowRect(GetDlgItem(hDlg, IDC_L_BET_LIST), &rect);
		BetList::maxDisplayedItemCnt = (rect.bottom - rect.top - 4) / listItemHeight;
		GetWindowRect(GetDlgItem(hDlg, IDC_MOVE_SPIN), &rect);
		X_MOVE = rect.left - rect.right;
	}

	resetButton.attach(GetDlgItem(hDlg, IDC_RESET_BUTTON));
	hTotalInvestText = GetDlgItem(hDlg, IDC_TOTAL_INVEST_TEXT);
	hCurrentProfitText[0] = GetDlgItem(hDlg, IDC_L_CURRENT_PROFIT_TEXT);
	hCurrentProfitText[1] = GetDlgItem(hDlg, IDC_R_CURRENT_PROFIT_TEXT);
	haveClosingCheck = GetDlgItem(hDlg, IDC_HAVE_CLOSING_CHECK);
	SendMessage(haveClosingCheck, WM_UPDATEUISTATE, MAKEWPARAM(UIS_SET, UISF_HIDEFOCUS), 0);
	SetWindowSubclass(haveClosingCheck, buttonSubclassProc, 0, 0);
	hMoveSpin = GetDlgItem(hDlg, IDC_MOVE_SPIN);
	allBoughtButton.attach(GetDlgItem(hDlg, IDC_ALL_BOUGHT_BUTTON));
	SetWindowSubclass(allBoughtButton.getHwnd(),
		[](HWND button, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)->LRESULT
		{
			LRESULT result = buttonSubclassProc(button, msg, wParam, lParam, uIdSubclass, dwRefData);
			if (msg == WM_LBUTTONUP)
			{
				SendMessage(GetParent(button), WM_COMMAND, MAKEWPARAM(IDC_ALL_BOUGHT_BUTTON, BN_KILLFOCUS), 0);
			}
			return result;
		},
		0, 0);
	boughtEdit.attach(GetDlgItem(hDlg, IDC_CHANGE_BOUGHT_EDIT));
	betList[0].attach(GetDlgItem(hDlg, IDC_L_BET_LIST), hMoveSpin, allBoughtButton.getHwnd(), &boughtEdit);
	betList[1].attach(GetDlgItem(hDlg, IDC_R_BET_LIST), hMoveSpin, allBoughtButton.getHwnd(), &boughtEdit);
	for (int i = 0; i < 10; i++)
	{
		hBankerBetSelector[i] = GetDlgItem(hDlg, IDC_L_BANKER_SELECTOR + i);
		SendMessage(hBankerBetSelector[i], WM_UPDATEUISTATE, MAKEWPARAM(UIS_SET, UISF_HIDEFOCUS), 0);
		SetWindowSubclass(hBankerBetSelector[i], buttonSubclassProc, 0, 0);
		oddsEdit[i].attach(GetDlgItem(hDlg, IDC_L_BANKER_ODDS_EDIT + i));
	}
	amountEdit[0].attach(GetDlgItem(hDlg, IDC_L_AMOUNT_EDIT));

	amountEdit[1].attach(GetDlgItem(hDlg, IDC_R_AMOUNT_EDIT));
	for (int i = 0; i < 8; i++)
	{
		addAmountButton[i].attach(GetDlgItem(hDlg, IDC_L_ADD_AMOUNT_BUTTON1 + i));
	}
	clearAmountButton[0].attach(GetDlgItem(hDlg, IDC_L_CLEAR_AMOUNT_BUTTON));
	clearAmountButton[1].attach(GetDlgItem(hDlg, IDC_R_CLEAR_AMOUNT_BUTTON));
	addButton[0].attach(GetDlgItem(hDlg, IDC_L_ADD_BUTTON));
	addButton[1].attach(GetDlgItem(hDlg, IDC_R_ADD_BUTTON));
	resultList[0].attach(GetDlgItem(hDlg, IDC_BALANCE_RESULT_LIST));
	confirmButton[0].attach(GetDlgItem(hDlg, IDC_BALANCE_CONFIRM_BUTTON));
	for (int i = 0; i < 8; i++)
	{
		hReferenceOddsText[i] = GetDlgItem(hDlg, IDC_L_RECOMMEND_BANKER_ODDS_TEXT + i);
	}
	initialAmountEdit.attach(GetDlgItem(hDlg, IDC_INITIAL_AMOUNT_EDIT));
	hWinProbSideLeftSelector = GetDlgItem(hDlg, IDC_L_WIN_PROBABILTY_SIDE_SELECTOR);
	SendMessage(hWinProbSideLeftSelector, WM_UPDATEUISTATE, MAKEWPARAM(UIS_SET, UISF_HIDEFOCUS), 0);
	SetWindowSubclass(hWinProbSideLeftSelector, buttonSubclassProc, 0, 0);
	SendMessage(GetDlgItem(hDlg, IDC_R_WIN_PROBABILTY_SIDE_SELECTOR), WM_UPDATEUISTATE, MAKEWPARAM(UIS_SET, UISF_HIDEFOCUS), 0);
	SetWindowSubclass(GetDlgItem(hDlg, IDC_R_WIN_PROBABILTY_SIDE_SELECTOR), buttonSubclassProc, 0, 0);
	winProbEdit.attach(GetDlgItem(hDlg, IDC_WIN_PROBABILITY_EDIT));
	winProbErrorEdit.attach(GetDlgItem(hDlg, IDC_WIN_PROBABILITY_ERROR_EDIT));
	winProbCalculatorButton.attach(GetDlgItem(hDlg, IDC_WIN_PROBABILITY_CALCULATOR_BUTTON));
	resultList[1].attach(GetDlgItem(hDlg, IDC_L_RESULT_LIST));
	resultList[2].attach(GetDlgItem(hDlg, IDC_R_RESULT_LIST));
	confirmButton[1].attach(GetDlgItem(hDlg, IDC_L_CONFIRM_BUTTON));
	confirmButton[2].attach(GetDlgItem(hDlg, IDC_R_CONFIRM_BUTTON));


	TCHAR resetTipText[] = _T("重置当前竞猜");
	createToolTip(resetButton.getHwnd(), hDlg, resetTipText);
	SendMessage(haveClosingCheck, BM_SETCHECK, config.defaultClosing, 0);
	for (int i = 0; i < 10; i += 2)
	{
		SendMessage(hBankerBetSelector[i], BM_SETCHECK, 1, 0);
	}
	for (int i = 0; i < 10; i++)
	{
		oddsEdit[i].setTextLimit(3);
		oddsEdit[i].setText(_T("0.1"));
	}
	for (int i = 0; i < 2; i++)
	{
		amountEdit[i].setTextLimit(7);
		RECT rect;
		amountEdit[i].getRect(&rect);
		rect.right -= 16 * xScale;
		amountEdit[i].setRectNP(&rect);
	}
	for (int i = 0; i < 4; i++)
	{
		TCHAR str[6];
		_itow(config.fastAddedAmount[i], str, 10);
		addAmountButton[i].setText(str);
		addAmountButton[i + 4].setText(str);
	}

	resetButton.setIcon(hResetIcon);
	clearAmountButton[0].setIcon(hClearIcon);
	clearAmountButton[0].setBkgBrush((HBRUSH)GetStockObject(WHITE_BRUSH));
	clearAmountButton[1].setIcon(hClearIcon);
	clearAmountButton[1].setBkgBrush((HBRUSH)GetStockObject(WHITE_BRUSH));
	allBoughtButton.setIcon(hTickIcon);
	winProbCalculatorButton.setIcon(hCalculatorIcon);
	boughtEdit.setTextLimit(7);
	initialAmountEdit.setTextLimit(14);

	SendMessage(hWinProbSideLeftSelector, BM_SETCHECK, 1, 0);
	winProbEdit.setTextLimit(4);
	winProbErrorEdit.setTextLimit(2);
	TCHAR str[3];
	_itow((int)round(config.defaultProbError * 100), str, 10);
	winProbErrorEdit.setText(str);
	winProbError = config.defaultProbError;

	TCHAR winProbCalculatorTipText[] = _T("加载胜率计算器");
	hWinProbCalculatorTip = createToolTip(winProbCalculatorButton.getHwnd(), hDlg, winProbCalculatorTipText);

	return INT_PTR(TRUE);
}

INT_PTR BetTabDlg::dlgProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case BPC_CONNECTED:
		if (hProbabilityCalculator == nullptr && wParam != NULL)
		{
			hProbabilityCalculator = (HWND)wParam;
			hProbabilityCalculatorIcon = CopyIcon((HICON)SendMessage(hProbabilityCalculator, WM_GETICON, ICON_BIG, 0));
			winProbCalculatorButton.setIcon(hProbabilityCalculatorIcon);
			TCHAR winProbCalculatorTipText[] = _T("断开胜率计算器");
			setToolTipText(hWinProbCalculatorTip, winProbCalculatorButton.getHwnd(), hDlg, winProbCalculatorTipText);
		}
		return INT_PTR(TRUE);
	case BPC_DISCONNECT:
		if (hProbabilityCalculator != nullptr && hProbabilityCalculator == (HWND)wParam)
		{
			disconnectCalculator();
		}
		return INT_PTR(TRUE);
	case BPC_PROBABILITY:
		if (hProbabilityCalculator != nullptr && hProbabilityCalculator == (HWND)wParam)
		{
			double probability = *(double*)&lParam;
			int probabitly4decimal = round(10000 * probability);
			if (probabitly4decimal > 0 && probabitly4decimal < 10000)
			{
				TCHAR str[5];
				_stprintf(str, _T("%04d"), probabitly4decimal);
				winProbEdit.setText(str);
				updateWinProb();
			}
		}
		return INT_PTR(TRUE);
	case WM_ERASEBKGND:
		{
			HBRUSH brush = GetSysColorBrush(CTLCOLOR_DLG);
			FillRect((HDC)wParam, &rcErase1, brush);
			FillRect((HDC)wParam, &rcErase2, brush);
			GetWindowRect(hMoveSpin, &rcErase1);
			MapWindowRect(HWND_DESKTOP, hDlg, &rcErase1);
			GetWindowRect(allBoughtButton.getHwnd(), &rcErase2);
			MapWindowRect(HWND_DESKTOP, hDlg, &rcErase2);
			return INT_PTR(TRUE);
		}
	case WM_CTLCOLORSTATIC:
		if ((HWND)lParam == hCurrentProfitText[0] && model.getProfit(1) - model.getProfit(0) > MIN_AMOUNT ||
			(HWND)lParam == hCurrentProfitText[1] && model.getProfit(0) - model.getProfit(1) > MIN_AMOUNT)
		{
			HDC hDC = (HDC)wParam;
			SetTextColor(hDC, RGB(255, 0, 0));
			SetBkColor(hDC, GetSysColor(CTLCOLOR_DLG));
			return (INT_PTR)GetSysColorBrush(CTLCOLOR_DLG);
		}
		break;
	case WM_KEYDOWN:
		switch (LOWORD(wParam))
		{
		case VK_UP:
			switch (GetDlgCtrlID(GetFocus()))
			{
			case IDC_L_AMOUNT_EDIT:
				oddsEdit[SendMessage(hBankerBetSelector[1], BM_GETCHECK, 0, 0)].oddsUp();
				return INT_PTR(TRUE);
			case IDC_R_AMOUNT_EDIT:
				oddsEdit[2 + SendMessage(hBankerBetSelector[3], BM_GETCHECK, 0, 0)].oddsUp();
				return INT_PTR(TRUE);
			}
		case VK_DOWN:
			switch (GetDlgCtrlID(GetFocus()))
			{
			case IDC_L_AMOUNT_EDIT:
				oddsEdit[SendMessage(hBankerBetSelector[1], BM_GETCHECK, 0, 0)].oddsDown();
				return INT_PTR(TRUE);
			case IDC_R_AMOUNT_EDIT:
				oddsEdit[2 + SendMessage(hBankerBetSelector[3], BM_GETCHECK, 0, 0)].oddsDown();
				return INT_PTR(TRUE);
			}
		}
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case ID_CONFIRM:
			switch (GetDlgCtrlID(GetFocus()))
			{
			case IDC_L_BANKER_ODDS_EDIT:
			case IDC_L_BET_ODDS_EDIT:
				SetFocus(amountEdit[LEFT_SIDE].getHwnd());
				return INT_PTR(TRUE);
			case IDC_R_BANKER_ODDS_EDIT:
			case IDC_R_BET_ODDS_EDIT:
				SetFocus(amountEdit[RIGHT_SIDE].getHwnd());
				return INT_PTR(TRUE);
			case IDC_L_AMOUNT_EDIT:
				add(LEFT_SIDE);
				return INT_PTR(TRUE);
			case IDC_R_AMOUNT_EDIT:
				add(RIGHT_SIDE);
				return INT_PTR(TRUE);
			case IDC_CHANGE_BOUGHT_EDIT:
				SetFocus(betList[selSide].getHwnd());
				return INT_PTR(TRUE);
			case IDC_BALANCE_AIM_BANKER_ODDS_EDIT:
			case IDC_BALANCE_AIM_BET_ODDS_EDIT:
				calcBalanceAimAmount();
				return INT_PTR(TRUE);
			case IDC_INITIAL_AMOUNT_EDIT:
				updateInitialAmount();
				return INT_PTR(TRUE);
			case IDC_WIN_PROBABILITY_EDIT:
				updateWinProb();
				return INT_PTR(TRUE);
			case IDC_WIN_PROBABILITY_ERROR_EDIT:
				updateWinProbError();
				return INT_PTR(TRUE);
			case IDC_L_AIM_BANKER_ODDS_EDIT:
			case IDC_L_AIM_BET_ODDS_EDIT:
				calcAimAmount(LEFT_SIDE);
				return INT_PTR(TRUE);
			case IDC_R_AIM_BANKER_ODDS_EDIT:
			case IDC_R_AIM_BET_ODDS_EDIT:
				calcAimAmount(RIGHT_SIDE);
				return INT_PTR(TRUE);
			}
			return INT_PTR(TRUE);
		case ID_CANCEL:
			switch (GetDlgCtrlID(GetFocus()))
			{
			case IDC_CHANGE_BOUGHT_EDIT:
				ShowWindow(boughtEdit.getHwnd(), SW_HIDE);
				SetFocus(betList[selSide].getHwnd());
				return INT_PTR(TRUE);
			}
			return INT_PTR(TRUE);
		case ID_DELETE:
			{
				auto result = betList[selSide].deleteSel();
				if (result.first)
				{
					model.deleteBet(selSide, result.second);
				}
				else
				{
					model.deleteBanker(selSide, result.second);
				}
				updateCurrentProfit();
				return INT_PTR(TRUE);
			}
		case IDC_RESET_BUTTON:
			if (!(betList[0].isEmpty() && betList[1].isEmpty()) && MessageBox(hDlg, _T("确定要重置当前竞猜吗？"), _T("bet"), MB_YESNO | MB_ICONQUESTION) == IDYES)
			{
				model.reset();
				for (int i = 0; i < 2; i++)
				{
					betList[i].resetContent();
				}
				updateCurrentProfit();
			}
			return INT_PTR(TRUE);
		case IDC_HAVE_CLOSING_CHECK:
			if (model.changeClosing())
			{
				updateCurrentProfit();
			}
			return INT_PTR(TRUE);
		case IDC_L_BET_LIST:
		case IDC_R_BET_LIST:
			switch (HIWORD(wParam))
			{
			case LBN_SETFOCUS:
				selSide = LOWORD(wParam) == IDC_R_BET_LIST;
				return INT_PTR(TRUE);
			}
			return INT_PTR(TRUE);
		case IDC_L_BANKER_SELECTOR:
		case IDC_L_BET_SELECTOR:
		case IDC_R_BANKER_SELECTOR:
		case IDC_R_BET_SELECTOR:
		case IDC_BALANCE_AIM_BANKER_SELECTOR:
		case IDC_BALANCE_AIM_BET_SELECTOR:
		case IDC_L_AIM_BANKER_SELECTOR:
		case IDC_L_AIM_BET_SELECTOR:
		case IDC_R_AIM_BANKER_SELECTOR:
		case IDC_R_AIM_BET_SELECTOR:
			SetFocus(oddsEdit[LOWORD(wParam) - IDC_L_BANKER_SELECTOR].getHwnd());
			return INT_PTR(TRUE);
		case IDC_L_BANKER_ODDS_EDIT:
		case IDC_L_BET_ODDS_EDIT:
		case IDC_R_BANKER_ODDS_EDIT:
		case IDC_R_BET_ODDS_EDIT:
		case IDC_BALANCE_AIM_BANKER_ODDS_EDIT:
		case IDC_BALANCE_AIM_BET_ODDS_EDIT:
		case IDC_L_AIM_BANKER_ODDS_EDIT:
		case IDC_L_AIM_BET_ODDS_EDIT:
		case IDC_R_AIM_BANKER_ODDS_EDIT:
		case IDC_R_AIM_BET_ODDS_EDIT:
			switch (HIWORD(wParam))
			{
			case EN_SETFOCUS:
				SendMessage(hBankerBetSelector[(LOWORD(wParam) - IDC_L_BANKER_ODDS_EDIT) ^ 1], BM_SETCHECK, 0, 0);
				SendMessage(hBankerBetSelector[LOWORD(wParam) - IDC_L_BANKER_ODDS_EDIT], BM_SETCHECK, 1, 0);
				return INT_PTR(TRUE);
			}
			break;
		case IDC_L_ADD_AMOUNT_BUTTON1:
		case IDC_L_ADD_AMOUNT_BUTTON2:
		case IDC_L_ADD_AMOUNT_BUTTON3:
		case IDC_L_ADD_AMOUNT_BUTTON4:
		case IDC_R_ADD_AMOUNT_BUTTON1:
		case IDC_R_ADD_AMOUNT_BUTTON2:
		case IDC_R_ADD_AMOUNT_BUTTON3:
		case IDC_R_ADD_AMOUNT_BUTTON4:
			{
				int side = LOWORD(wParam) > IDC_L_ADD_AMOUNT_BUTTON4;
				TCHAR str[8];
				amountEdit[side].getText(str, 8);
				int amount = _wtoi(str) + config.fastAddedAmount[(LOWORD(wParam) - IDC_L_ADD_AMOUNT_BUTTON1) & 3];
				if (amount > 9999999)
				{
					amount = 9999999;
				}
				_itow(amount, str, 10);
				amountEdit[side].setText(str);
				SetFocus(amountEdit[side].getHwnd());
				return INT_PTR(TRUE);
			}
		case IDC_L_CLEAR_AMOUNT_BUTTON:
		case IDC_R_CLEAR_AMOUNT_BUTTON:
			amountEdit[LOWORD(wParam) - IDC_L_CLEAR_AMOUNT_BUTTON].setText(_T(""));
			SetFocus(amountEdit[LOWORD(wParam) - IDC_L_CLEAR_AMOUNT_BUTTON].getHwnd());
			return INT_PTR(TRUE);
		case IDC_L_ADD_BUTTON:
		case IDC_R_ADD_BUTTON:
			add(LOWORD(wParam) - IDC_L_ADD_BUTTON);
			return INT_PTR(TRUE);
		case IDC_CHANGE_BOUGHT_EDIT:
			switch (HIWORD(wParam))
			{
			case EN_KILLFOCUS:
				{
					if (!IsWindowVisible(boughtEdit.getHwnd()))
					{
						return INT_PTR(TRUE);
					}
					TCHAR str[8];
					boughtEdit.getText(str, 8);
					ShowWindow(boughtEdit.getHwnd(), SW_HIDE);
					if (str[0] == '\0')
					{
						betList[selSide].setCurSel(-1);
						return INT_PTR(TRUE);
					}
					int lineIdx = betList[selSide].getCurSel();
					const Banker& banker = model.changeBought(selSide, lineIdx - betList[selSide].getBetsSize() - 5, _wtoi(str));
					betList[selSide].updateBanker(lineIdx, banker.show, banker.maxBought == banker.bought ? GetSysColor(COLOR_WINDOWTEXT) : RGB(255, 0, 0));
					updateCurrentProfit();
					return INT_PTR(TRUE);
				}
				break;
			}
		case IDC_ALL_BOUGHT_BUTTON:
			switch (HIWORD(wParam))
			{
			case BN_CLICKED:
				{
					int lineIdx = betList[selSide].getCurSel();
					betList[selSide].updateBanker(lineIdx, model.allBought(selSide, lineIdx - betList[selSide].getBetsSize() - 5).show);
					SetFocus(betList[selSide].getHwnd());
					updateCurrentProfit();
					return INT_PTR(TRUE);
				}
			case BN_KILLFOCUS:
				SetFocus(betList[selSide].getHwnd());
				return INT_PTR(TRUE);
			}
			break;
		case IDC_BALANCE_CONFIRM_BUTTON:
			{
				calcBalanceAimAmount();
				return INT_PTR(TRUE);
			}
		case IDC_INITIAL_AMOUNT_EDIT:
			switch (HIWORD(wParam))
			{
			case EN_KILLFOCUS:
				updateInitialAmount();
				return INT_PTR(TRUE);
			}
			break;
		case IDC_L_WIN_PROBABILTY_SIDE_SELECTOR:
		case IDC_R_WIN_PROBABILTY_SIDE_SELECTOR:
			if (winProbSide == SendMessage(hWinProbSideLeftSelector, BM_GETCHECK, 0, 0))
			{
				winProbSide = !winProbSide;
				if (winProb != 0)
				{
					winProb = 1 - winProb;
					updateMinOdds();
				}
			}
			return INT_PTR(TRUE);
		case IDC_WIN_PROBABILITY_EDIT:
			switch (HIWORD(wParam))
			{
			case EN_KILLFOCUS:
				updateWinProb();
				return INT_PTR(TRUE);
			}
			break;
		case IDC_WIN_PROBABILITY_ERROR_EDIT:
			switch (HIWORD(wParam))
			{
			case EN_KILLFOCUS:
				updateWinProbError();
				return INT_PTR(TRUE);
			}
			break;
		case IDC_WIN_PROBABILITY_CALCULATOR_BUTTON:
			if (hProbabilityCalculator == nullptr)
			{
				// CoCreate the File Open Dialog object.
				IFileDialog* pfd = nullptr;
				HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog,
					nullptr,
					CLSCTX_INPROC_SERVER,
					IID_PPV_ARGS(&pfd));
				if (SUCCEEDED(hr))
				{
					// Set the file types to display only. 
					// Notice that this is a 1-based array.
					COMDLG_FILTERSPEC c_rgSaveTypes = { _T(""),_T("*.exe") };
					hr = pfd->SetFileTypes(1, &c_rgSaveTypes);
					if (SUCCEEDED(hr))
					{
						// Show the dialog
						hr = pfd->Show(hDlg);
						if (SUCCEEDED(hr))
						{
							// Obtain the result once the user clicks 
							// the 'Open' button.
							// The result is an IShellItem object.
							IShellItem* psiResult;
							hr = pfd->GetResult(&psiResult);
							if (SUCCEEDED(hr))
							{
								PTSTR pszFilePath = nullptr;
								hr = psiResult->GetDisplayName(SIGDN_FILESYSPATH,
									&pszFilePath);
								if (SUCCEEDED(hr))
								{
									TCHAR cmdLine[200];
									_stprintf(cmdLine, _T("bet_probability_calculator "
										"HWND=%p "
										"connect_message=%d "
										"disconnect_message=%d "
										"probability_message=%d"),
										hDlg,
										BPC_CONNECTED,
										BPC_DISCONNECT,
										BPC_PROBABILITY);
									ShellExecute(nullptr, nullptr, pszFilePath, cmdLine, nullptr, SW_SHOWNORMAL);
									CoTaskMemFree(pszFilePath);
								}
								psiResult->Release();
							}
						}
					}
					pfd->Release();
				}
			}
			else
			{
				PostMessage(hProbabilityCalculator, BPC_DISCONNECT, 0, 0);
				disconnectCalculator();
			}
			return INT_PTR(TRUE);
		case IDC_L_CONFIRM_BUTTON:
		case IDC_R_CONFIRM_BUTTON:
			calcAimAmount(LOWORD(wParam) - IDC_L_CONFIRM_BUTTON);
			return INT_PTR(TRUE);
		}
		break;
	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->idFrom)
		{
		case IDC_L_BANKER_ODDS_SPIN:
		case IDC_L_BET_ODDS_SPIN:
		case IDC_R_BANKER_ODDS_SPIN:
		case IDC_R_BET_ODDS_SPIN:
		case IDC_BALANCE_AIM_BANKER_ODDS_SPIN:
		case IDC_BALANCE_AIM_BET_ODDS_SPIN:
		case IDC_L_AIM_BANKER_ODDS_SPIN:
		case IDC_L_AIM_BET_ODDS_SPIN:
		case IDC_R_AIM_BANKER_ODDS_SPIN:
		case IDC_R_AIM_BET_ODDS_SPIN:
			if (((LPNMUPDOWN)lParam)->iDelta < 0)
			{
				oddsEdit[((LPNMHDR)lParam)->idFrom - IDC_L_BANKER_ODDS_SPIN].oddsUp(-0.1 * ((LPNMUPDOWN)lParam)->iDelta);
			}
			else
			{
				oddsEdit[((LPNMHDR)lParam)->idFrom - IDC_L_BANKER_ODDS_SPIN].oddsDown(0.1 * ((LPNMUPDOWN)lParam)->iDelta);
			}
			SetFocus(oddsEdit[((LPNMHDR)lParam)->idFrom - IDC_L_BANKER_ODDS_SPIN].getHwnd());
			return INT_PTR(TRUE);
		case IDC_MOVE_SPIN:
			{
				int itemIdx = betList[selSide].moveSel(((LPNMUPDOWN)lParam)->iDelta < 0);
				if (itemIdx == -1)
				{
					return INT_PTR(TRUE);
				}
				if (IsWindowVisible(allBoughtButton.getHwnd()))
				{
					model.moveBackBanker(selSide, itemIdx);
				}
				else
				{
					model.moveBackBet(selSide, itemIdx);
				}
				return INT_PTR(TRUE);
			}
		}
		break;
	case WM_DESTROY:
		if (hProbabilityCalculator != nullptr)
		{
			PostMessage(hProbabilityCalculator, BPC_DISCONNECT, 0, 0);
			DeleteObject(hProbabilityCalculatorIcon);
		}
		break;
	}
	return INT_PTR(FALSE);
}

void BetTabDlg::updateCurrentProfit()
{
	TCHAR str[20];
	_stprintf(str, _T("%lld"), model.getTotalInvest());
	SetWindowText(hTotalInvestText, str);
	_stprintf(str, _T("%lld"), model.getProfit(0));
	SetWindowText(hCurrentProfitText[0], str);
	_stprintf(str, _T("%lld"), model.getProfit(1));
	SetWindowText(hCurrentProfitText[1], str);
	SendMessage(resultList[0].getHwnd(), LB_RESETCONTENT, 0, 0);
	updateMinOdds();
}

void BetTabDlg::updateMinOdds()
{
	SendMessage(resultList[1].getHwnd(), LB_RESETCONTENT, 0, 0);
	SendMessage(resultList[2].getHwnd(), LB_RESETCONTENT, 0, 0);
	if (initialAmount < model.getTotalInvest() + MIN_AMOUNT || winProb == 0)
	{
		for (int i = 0; i < 8; i++)
		{
			SetWindowText(hReferenceOddsText[i], _T("N/A"));
		}
		return;
	}
	double referenceOdds[8]{};
	model.calcReferenceOdds(initialAmount, winProb, winProbError, referenceOdds);
	for (int i = 0; i < 8; i++)
	{
		if (referenceOdds[i] == 0)
		{
			SetWindowText(hReferenceOddsText[i], _T("-"));
		}
		else
		{
			TCHAR str[4];
			_stprintf(str, _T("%.1f"), referenceOdds[i]);
			SetWindowText(hReferenceOddsText[i], str);
		}
	}
}

void BetTabDlg::add(int side)
{
	TCHAR str[8];
	amountEdit[side].getText(str, 8);
	int amount = _wtoi(str);
	if (amount == 0)
	{
		amountEdit[side].setSel(0, -1);
		SetFocus(amountEdit[side].getHwnd());
		return;
	}
	if (SendMessage(hBankerBetSelector[2 * side + 1], BM_GETCHECK, 0, 0))
	{
		Bet bet(oddsEdit[2 * side + 1].getOdds(), amount);
		model.addBet(side, bet);
		betList[side].addBet(bet.show);
	}
	else
	{
		Banker banker(oddsEdit[2 * side].getOdds(), amount);
		model.addBanker(side, banker);
		betList[side].addBanker(banker.show);
	}
	SetFocus(amountEdit[side].getHwnd());
	updateCurrentProfit();
}

void BetTabDlg::calcBalanceAimAmount()
{
	bool isBet = SendMessage(hBankerBetSelector[5], BM_GETCHECK, 0, 0);
	TCHAR str[30];
	_stprintf(str, _T("%s %0.1f"), isBet ? _T("下注") : _T("庄家"), oddsEdit[4 + isBet].getOdds());
	if (SendMessage(resultList[0].getHwnd(), LB_FINDSTRING, 0, (LPARAM)str) == -1)
	{
		auto result = model.calcAimAmountBalance(isBet, isBet ? oddsEdit[5].getOdds() : oddsEdit[4].getOdds());
		if (result.first <= MIN_AMOUNT)
		{
			lstrcat(str, _T("     收益已平衡"));
		}
		else
		{
			if (result.first < 100000000LL)
			{
				_stprintf(&str[6], _T(" %8lld"), result.first);
			}
			else if (result.first < 10000000000)
			{
				_stprintf(&str[6], _T(" %6lld万"), (long long)round(result.first * 0.0001));
			}
			else if (result.first < 1000000000000)
			{
				int decimal = 1;
				for (long long i = 100000000000; i > result.first; decimal++, i /= 10);
				_stprintf(&str[6], _T(" %.*f亿"), decimal, result.first * 0.00000001);
			}
			else
			{
				_stprintf(&str[6], _T(" %6lld亿"), (long long)round(result.first * 0.00000001));
			}
			if (result.second < 1000000000LL && result.second > -100000000LL)
			{
				_stprintf(&str[15], _T(" %9lld"), result.second);
			}
			else if (result.second < 100000000000 && result.second > -1000000000)
			{
				_stprintf(&str[15], _T(" %7lld万"), (long long)round(result.second * 0.0001));
			}
			else if (result.second < 10000000000000 && result.second > -100000000000)
			{
				int decimal = 1;
				for (long long i = 100000000000; i * 10 > result.second && -i < result.second; decimal++, i /= 10);
				_stprintf(&str[15], _T(" %.*f亿"), decimal, result.second * 0.00000001);
			}
			else
			{
				_stprintf(&str[15], _T(" %7lld亿"), (long long)round(result.second * 0.00000001));
			}
		}
		int index = resultList[0].addString(str, DT_VCENTER);
	}
	SendMessage(resultList[0].getHwnd(), LB_SELECTSTRING, 0, (LPARAM)str);
	SetFocus(oddsEdit[4 + isBet].getHwnd());
}

void BetTabDlg::updateInitialAmount()
{
	TCHAR str[20];
	initialAmountEdit.getText(str, 20);
	long long newAmount = _wtoll(str);
	if (newAmount != initialAmount)
	{
		initialAmount = newAmount;
		updateMinOdds();
	}
}

void BetTabDlg::updateWinProb()
{
	TCHAR str[5];
	winProbEdit.getText(str, 5);
	double result = 0;
	for (int i = lstrlen(str) - 1; i >= 0; i--)
	{
		int c = str[i] - '0';
		result = (result + c) * 0.1;
	}
	if (result != 0 && winProbSide)
	{
		result = 1 - result;
	}
	if (result != winProb)
	{
		winProb = result;
		updateMinOdds();
	}
}

void BetTabDlg::updateWinProbError()
{
	TCHAR str[3];
	winProbErrorEdit.getText(str, 3);
	double newError = _wtoi(str) / 100.0;
	if (newError != winProbError)
	{
		winProbError = newError;
		updateMinOdds();
	}
}

void BetTabDlg::disconnectCalculator()
{
	hProbabilityCalculator = nullptr;
	winProbCalculatorButton.setIcon(hCalculatorIcon);
	DeleteObject(hProbabilityCalculatorIcon);
	hProbabilityCalculatorIcon = nullptr;
	TCHAR winProbCalculatorTipText[] = _T("加载胜率计算器");
	setToolTipText(hWinProbCalculatorTip, winProbCalculatorButton.getHwnd(), hDlg, winProbCalculatorTipText);
}

void BetTabDlg::calcAimAmount(int side)
{
	bool isBet = SendMessage(hBankerBetSelector[2 * side + 7], BM_GETCHECK, 0, 0);
	if (initialAmount < model.getTotalInvest() + MIN_AMOUNT)
	{
		if (SendMessage(resultList[side + 1].getHwnd(), LB_FINDSTRING, 0, (LPARAM)_T("初始数量过低")) == -1)
		{
			resultList[side + 1].addString(_T("初始数量过低"), DT_VCENTER | DT_CENTER);
		}
		resultList[side + 1].setCurSel(0);
		initialAmountEdit.setSel(0, -1);
		SetFocus(initialAmountEdit.getHwnd());
		return;
	}
	if (winProb == 0)
	{
		if (SendMessage(resultList[side + 1].getHwnd(), LB_FINDSTRING, 0, (LPARAM)_T("输入胜率为0")) == -1)
		{
			resultList[side + 1].addString(_T("输入胜率为0"), DT_VCENTER | DT_CENTER);
		}
		resultList[side + 1].setCurSel(0);
		winProbEdit.setSel(0, -1);
		SetFocus(winProbEdit.getHwnd());
		return;
	}
	TCHAR str[20];
	_stprintf(str, _T("%s %0.1f"), isBet ? _T("下注") : _T("庄家"), oddsEdit[6 + 2 * side + isBet].getOdds());
	if (SendMessage(resultList[side + 1].getHwnd(), LB_FINDSTRING, 0, (LPARAM)str) == -1)
	{
		long long aimAmount = model.calcAimAmountProb(initialAmount, winProb, winProbError, side, isBet, oddsEdit[6 + 2 * side + isBet].getOdds());
		if (aimAmount < 10000000LL)
		{
			_stprintf(&str[6], _T(" %7lld"), aimAmount);
		}
		else if (aimAmount < 1000000000)
		{
			_stprintf(&str[6], _T(" %5lld万"), (long long)round(aimAmount * 0.0001));
		}
		else if (aimAmount < 100000000000)
		{
			int decimal = 1;
			for (long long i = 10000000000; i > aimAmount; decimal++, i /= 10);
			_stprintf(&str[6], _T(" %.*f亿"), decimal, aimAmount * 0.00000001);
		}
		else
		{
			_stprintf(&str[6],  _T(" %5lld亿"), (long long)round(aimAmount * 0.00000001));
		}
		resultList[side + 1].addString(str, DT_VCENTER);
	}
	SendMessage(resultList[side + 1].getHwnd(), LB_SELECTSTRING, 0, (LPARAM)str);
	SetFocus(oddsEdit[6 + 2 * side + isBet].getHwnd());
}