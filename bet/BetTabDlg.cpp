#include "framework.h"
#include "BetTabDlg.h"

#include "tooltip.h"
#include "common.h"
#include "CutInputDlg.h"

#include <CommCtrl.h>
#include <Vsstyle.h>
#include <uxtheme.h>
#include <shobjidl_core.h>
#include <shellapi.h>
#include <windowsx.h>

#define MAX_BET_COUNT 50000

#define BPC_CONNECTED (WM_APP)
#define BPC_DISCONNECT (WM_APP+1)
#define BPC_PROBABILITY (WM_APP+2)

#define LEFT_SIDE 0
#define RIGHT_SIDE 1

HICON BetTabDlg::hResetIcon;
HICON BetTabDlg::hClearIcon;
HICON BetTabDlg::hTickIcon;
HICON BetTabDlg::hCalculatorIcon;

BetTabDlg::BetTabDlg(double cut) :model(cut)
{
	nIDTemplate = IDD_BET_TAB_DIALOG;
}

INT_PTR BetTabDlg::initDlg(HWND hDlg)
{
	Dialog::initDlg(hDlg);

	resetButton.attach(GetDlgItem(hDlg, IDC_RESET_BUTTON));
	hTotalInvestText = GetDlgItem(hDlg, IDC_TOTAL_INVEST_TEXT);
	hCurrentProfitTexts[0] = GetDlgItem(hDlg, IDC_L_CURRENT_PROFIT_TEXT);
	hCurrentProfitTexts[1] = GetDlgItem(hDlg, IDC_R_CURRENT_PROFIT_TEXT);
	hHaveClosingCheck = GetDlgItem(hDlg, IDC_HAVE_CLOSING_CHECK);
	SendMessage(hHaveClosingCheck, WM_UPDATEUISTATE, MAKEWPARAM(UIS_SET, UISF_HIDEFOCUS), 0);
	SetWindowSubclass(hHaveClosingCheck, buttonSubclassProc, 0, 0);
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
		}, 0, 0);
	boughtEdit.attach(GetDlgItem(hDlg, IDC_CHANGE_BOUGHT_EDIT));
	betLists[0].attach(GetDlgItem(hDlg, IDC_L_BET_LIST), hMoveSpin, allBoughtButton.getHwnd(), &boughtEdit);
	betLists[1].attach(GetDlgItem(hDlg, IDC_R_BET_LIST), hMoveSpin, allBoughtButton.getHwnd(), &boughtEdit);
	for (int i = 0; i < 10; i++)
	{
		hBankerBetSelectors[i] = GetDlgItem(hDlg, IDC_L_BANKER_SELECTOR + i);
		SendMessage(hBankerBetSelectors[i], WM_UPDATEUISTATE, MAKEWPARAM(UIS_SET, UISF_HIDEFOCUS), 0);
		SetWindowSubclass(hBankerBetSelectors[i], buttonSubclassProc, 0, 0);
		oddsEdits[i].attach(GetDlgItem(hDlg, IDC_L_BANKER_ODDS_EDIT + i));
	}
	amountEdits[0].attach(GetDlgItem(hDlg, IDC_L_AMOUNT_EDIT), oddsEdits[0].getHwnd(), oddsEdits[1].getHwnd(), hBankerBetSelectors[0]);
	amountEdits[1].attach(GetDlgItem(hDlg, IDC_R_AMOUNT_EDIT), oddsEdits[2].getHwnd(), oddsEdits[3].getHwnd(), hBankerBetSelectors[2]);
	for (int i = 0; i < 8; i++)
	{
		addAmountButtons[i].attach(GetDlgItem(hDlg, IDC_L_ADD_AMOUNT_BUTTON1 + i));
	}
	addButtons[0].attach(GetDlgItem(hDlg, IDC_L_ADD_BUTTON));
	addButtons[1].attach(GetDlgItem(hDlg, IDC_R_ADD_BUTTON));
	resultLists[0].attach(GetDlgItem(hDlg, IDC_BALANCE_RESULT_LIST));
	confirmButtons[0].attach(GetDlgItem(hDlg, IDC_BALANCE_CONFIRM_BUTTON));
	for (int i = 0; i < 8; i++)
	{
		hReferenceOddsTexts[i] = GetDlgItem(hDlg, IDC_L_REC_BANKER_ODDS_TEXT + i);
	}
	initialAmountEdit.attach(GetDlgItem(hDlg, IDC_INITIAL_AMOUNT_EDIT));
	hRemainingAmountText = GetDlgItem(hDlg, IDC_REMAINING_AMOUNT_TEXT);
	hWinProbSideLeftSelector = GetDlgItem(hDlg, IDC_L_WIN_PROB_SIDE_SELECTOR);
	SendMessage(hWinProbSideLeftSelector, WM_UPDATEUISTATE, MAKEWPARAM(UIS_SET, UISF_HIDEFOCUS), 0);
	SetWindowSubclass(hWinProbSideLeftSelector, buttonSubclassProc, 0, 0);
	SendMessage(GetDlgItem(hDlg, IDC_R_WIN_PROB_SIDE_SELECTOR), WM_UPDATEUISTATE, MAKEWPARAM(UIS_SET, UISF_HIDEFOCUS), 0);
	SetWindowSubclass(GetDlgItem(hDlg, IDC_R_WIN_PROB_SIDE_SELECTOR), buttonSubclassProc, 0, 0);
	winProbEdit.attach(GetDlgItem(hDlg, IDC_WIN_PROB_EDIT));
	winProbErrorEdit.attach(GetDlgItem(hDlg, IDC_WIN_PROB_ERROR_EDIT));
	winProbCalculatorButton.attach(GetDlgItem(hDlg, IDC_WIN_PROB_CALCULATOR_BUTTON));
	resultLists[1].attach(GetDlgItem(hDlg, IDC_L_RESULT_LIST));
	resultLists[2].attach(GetDlgItem(hDlg, IDC_R_RESULT_LIST));
	confirmButtons[1].attach(GetDlgItem(hDlg, IDC_L_CONFIRM_BUTTON));
	confirmButtons[2].attach(GetDlgItem(hDlg, IDC_R_CONFIRM_BUTTON));

	TCHAR str[7];
	_stprintf(str, _T("%.2f%%"), 100 * (1 - model.getCut()));
	SetWindowText(GetDlgItem(hDlg, IDC_CUT_TEXT), str);

	Button_SetCheck(hHaveClosingCheck, config.defClosing);
	for (int i = 0; i < 10; i += 2)
	{
		Button_SetCheck(hBankerBetSelectors[i], 1);
	}
	for (int i = 0; i < 10; i++)
	{
		oddsEdits[i].setText(_T("0.1"));
	}
	for (int i = 0; i < 4; i++)
	{
		TCHAR str[6];
		_itot(config.fastAddedAmount[i], str, 10);
		addAmountButtons[i].setText(str);
		addAmountButtons[i + 4].setText(str);
	}
	Edit_LimitText(amountEdits[0].getHwnd(), 7);
	Edit_LimitText(amountEdits[1].getHwnd(), 7);

	TCHAR resetTipText[] = _T("重置当前竞猜");
	createToolTip(resetButton.getHwnd(), hDlg, resetTipText);

	resetButton.setIcon(hResetIcon);
	allBoughtButton.setIcon(hTickIcon);
	winProbCalculatorButton.setIcon(hCalculatorIcon);

	Button_SetCheck(hWinProbSideLeftSelector, 1);
	_itot((int)round(config.defProbError * 100), str, 10);
	winProbErrorEdit.setText(str);
	winProbError = config.defProbError;

	TCHAR winProbCalculatorTipText[] = _T("加载胜率计算器");
	hWinProbCalculatorTip = createToolTip(winProbCalculatorButton.getHwnd(), hDlg, winProbCalculatorTipText);
	return (INT_PTR)TRUE;
}

INT_PTR BetTabDlg::dlgProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case BPC_CONNECTED:
		if (hProbCalculator == nullptr && wParam != NULL)
		{
			hProbCalculator = (HWND)wParam;
			hProbCalculatorIcon = CopyIcon((HICON)SendMessage(hProbCalculator, WM_GETICON, ICON_BIG, 0));
			winProbCalculatorButton.setIcon(hProbCalculatorIcon);
			TCHAR winProbCalculatorTipText[] = _T("断开胜率计算器");
			setToolTipText(hWinProbCalculatorTip, winProbCalculatorButton.getHwnd(), hDlg, winProbCalculatorTipText);
		}
		return (INT_PTR)TRUE;
	case BPC_DISCONNECT:
		if (hProbCalculator != nullptr && hProbCalculator == (HWND)wParam)
		{
			disconnectCalculator();
		}
		return (INT_PTR)TRUE;
	case BPC_PROBABILITY:
		if (hProbCalculator != nullptr && hProbCalculator == (HWND)wParam)
		{
			int probDecimal = round(10000 * *(double*)&lParam);
			if (probDecimal > 0 && probDecimal < 10000)
			{
				TCHAR str[5];
				_stprintf(str, _T("%04d"), probDecimal);
				winProbEdit.setText(str);
				updateWinProb();
			}
		}
		return (INT_PTR)TRUE;
	case WM_ERASEBKGND:
		{
			HBRUSH brush = GetSysColorBrush(COLOR_BTNFACE);
			FillRect((HDC)wParam, &rcErase1, brush);
			FillRect((HDC)wParam, &rcErase2, brush);
			GetWindowRect(hMoveSpin, &rcErase1);
			MapWindowRect(HWND_DESKTOP, hDlg, &rcErase1);
			GetWindowRect(allBoughtButton.getHwnd(), &rcErase2);
			MapWindowRect(HWND_DESKTOP, hDlg, &rcErase2);
			return (INT_PTR)TRUE;
		}
	case WM_CTLCOLORSTATIC:
		if ((HWND)lParam == hCurrentProfitTexts[LEFT_SIDE] && model.getProfit(RIGHT_SIDE) - model.getProfit(LEFT_SIDE) > 1 ||
			(HWND)lParam == hCurrentProfitTexts[RIGHT_SIDE] && model.getProfit(LEFT_SIDE) - model.getProfit(RIGHT_SIDE) > 1)
		{
			SetTextColor((HDC)wParam, RGB(255, 0, 0));
			SetBkColor((HDC)wParam, GetSysColor(COLOR_BTNFACE));
			return (INT_PTR)GetSysColorBrush(COLOR_BTNFACE);
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
				SetFocus(amountEdits[LEFT_SIDE].getHwnd());
				return (INT_PTR)TRUE;
			case IDC_R_BANKER_ODDS_EDIT:
			case IDC_R_BET_ODDS_EDIT:
				SetFocus(amountEdits[RIGHT_SIDE].getHwnd());
				return (INT_PTR)TRUE;
			case IDC_L_AMOUNT_EDIT:
				add(LEFT_SIDE);
				return (INT_PTR)TRUE;
			case IDC_R_AMOUNT_EDIT:
				add(RIGHT_SIDE);
				return (INT_PTR)TRUE;
			case IDC_CHANGE_BOUGHT_EDIT:
				SetFocus(betLists[selSide].getHwnd());
				return (INT_PTR)TRUE;
			case IDC_BALANCE_AIM_BANKER_ODDS_EDIT:
			case IDC_BALANCE_AIM_BET_ODDS_EDIT:
				calcBalanceAimAmount();
				return (INT_PTR)TRUE;
			case IDC_INITIAL_AMOUNT_EDIT:
				updateInitialAmount();
				return (INT_PTR)TRUE;
			case IDC_WIN_PROB_EDIT:
				updateWinProb();
				return (INT_PTR)TRUE;
			case IDC_WIN_PROB_ERROR_EDIT:
				updateWinProbError();
				return (INT_PTR)TRUE;
			case IDC_L_AIM_BANKER_ODDS_EDIT:
			case IDC_L_AIM_BET_ODDS_EDIT:
				calcAimAmount(LEFT_SIDE);
				return (INT_PTR)TRUE;
			case IDC_R_AIM_BANKER_ODDS_EDIT:
			case IDC_R_AIM_BET_ODDS_EDIT:
				calcAimAmount(RIGHT_SIDE);
				return (INT_PTR)TRUE;
			}
			return (INT_PTR)TRUE;
		case ID_CANCEL:
			switch (GetDlgCtrlID(GetFocus()))
			{
			case IDC_CHANGE_BOUGHT_EDIT:
				ShowWindow(boughtEdit.getHwnd(), SW_HIDE);
				SetFocus(betLists[selSide].getHwnd());
				return (INT_PTR)TRUE;
			}
			return (INT_PTR)TRUE;
		case ID_DELETE:
			{
				auto result = betLists[selSide].deleteSel();
				if (result.first)
				{
					model.deleteBet(selSide, result.second);
				}
				else
				{
					model.deleteBanker(selSide, result.second);
				}
				updateCurrentProfit();
				return (INT_PTR)TRUE;
			}
		case IDC_RESET_BUTTON:
			if (!(betLists[0].isEmpty() && betLists[1].isEmpty()) && MessageBox(hDlg, _T("确定要重置当前竞猜吗？"), _T("bet"), MB_YESNO | MB_ICONQUESTION) == IDYES)
			{
				model.reset();
				for (int i = 0; i < 2; i++)
				{
					betLists[i].resetContent();
				}
				updateCurrentProfit();
			}
			return (INT_PTR)TRUE;
		case IDC_HAVE_CLOSING_CHECK:
			if (model.changeClosing())
			{
				updateCurrentProfit();
			}
			return (INT_PTR)TRUE;
		case IDC_L_BET_LIST:
		case IDC_R_BET_LIST:
			switch (HIWORD(wParam))
			{
			case LBN_SETFOCUS:
				selSide = LOWORD(wParam) == IDC_R_BET_LIST;
				return (INT_PTR)TRUE;
			}
			return (INT_PTR)TRUE;
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
			SetFocus(oddsEdits[LOWORD(wParam) - IDC_L_BANKER_SELECTOR].getHwnd());
			return (INT_PTR)TRUE;
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
				Button_SetCheck(hBankerBetSelectors[(LOWORD(wParam) - IDC_L_BANKER_ODDS_EDIT) ^ 1], 0);
				Button_SetCheck(hBankerBetSelectors[LOWORD(wParam) - IDC_L_BANKER_ODDS_EDIT], 1);
				return (INT_PTR)TRUE;
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
				amountEdits[side].getText(str, 8);
				int amount = _ttoi(str) + config.fastAddedAmount[(LOWORD(wParam) - IDC_L_ADD_AMOUNT_BUTTON1) & 3];
				if (amount > 9999999)
				{
					amount = 9999999;
				}
				_itot(amount, str, 10);
				amountEdits[side].setText(str);
				SetFocus(amountEdits[side].getHwnd());
				return (INT_PTR)TRUE;
			}
		case IDC_L_ADD_BUTTON:
		case IDC_R_ADD_BUTTON:
			add(LOWORD(wParam) - IDC_L_ADD_BUTTON);
			return (INT_PTR)TRUE;
		case IDC_CHANGE_BOUGHT_EDIT:
			switch (HIWORD(wParam))
			{
			case EN_KILLFOCUS:
				{
					if (!IsWindowVisible(boughtEdit.getHwnd()))
					{
						return (INT_PTR)TRUE;
					}
					TCHAR str[8];
					boughtEdit.getText(str, 8);
					ShowWindow(boughtEdit.getHwnd(), SW_HIDE);
					if (str[0] == '\0')
					{
						betLists[selSide].setCurSel(-1);
						return (INT_PTR)TRUE;
					}
					int lineIdx = betLists[selSide].getCurSel();
					const Banker& banker = model.changeBought(selSide, lineIdx - betLists[selSide].getBetsSize() - 5, _ttoi(str));
					betLists[selSide].updateBanker(lineIdx, banker.show, banker.maxBought == banker.bought ? GetSysColor(COLOR_WINDOWTEXT) : RGB(255, 0, 0));
					updateCurrentProfit();
					return (INT_PTR)TRUE;
				}
				break;
			}
		case IDC_ALL_BOUGHT_BUTTON:
			switch (HIWORD(wParam))
			{
			case BN_CLICKED:
				{
					int lineIdx = betLists[selSide].getCurSel();
					betLists[selSide].updateBanker(lineIdx, model.allBought(selSide, lineIdx - betLists[selSide].getBetsSize() - 5).show);
					SetFocus(betLists[selSide].getHwnd());
					updateCurrentProfit();
					return (INT_PTR)TRUE;
				}
			case BN_KILLFOCUS:
				SetFocus(betLists[selSide].getHwnd());
				return (INT_PTR)TRUE;
			}
			break;
		case IDC_BALANCE_CONFIRM_BUTTON:
			{
				calcBalanceAimAmount();
				return (INT_PTR)TRUE;
			}
		case IDC_INITIAL_AMOUNT_EDIT:
			switch (HIWORD(wParam))
			{
			case EN_KILLFOCUS:
				updateInitialAmount();
				return (INT_PTR)TRUE;
			}
			break;
		case IDC_L_WIN_PROB_SIDE_SELECTOR:
		case IDC_R_WIN_PROB_SIDE_SELECTOR:
			if (winProbSide == Button_GetCheck(hWinProbSideLeftSelector))
			{
				winProbSide = !winProbSide;
				if (winProb != 0)
				{
					winProb = 1 - winProb;
					updateMinOdds();
				}
			}
			return (INT_PTR)TRUE;
		case IDC_WIN_PROB_EDIT:
			switch (HIWORD(wParam))
			{
			case EN_KILLFOCUS:
				updateWinProb();
				return (INT_PTR)TRUE;
			}
			break;
		case IDC_WIN_PROB_ERROR_EDIT:
			switch (HIWORD(wParam))
			{
			case EN_KILLFOCUS:
				updateWinProbError();
				return (INT_PTR)TRUE;
			}
			break;
		case IDC_WIN_PROB_CALCULATOR_BUTTON:
			if (hProbCalculator == nullptr)
			{
				IFileDialog* pfd = nullptr;
				HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd));
				if (SUCCEEDED(hr))
				{
					COMDLG_FILTERSPEC fileType = { _T(""),_T("*.exe") };
					pfd->SetFileTypes(1, &fileType);
					pfd->Show(hDlg);
					IShellItem* psiResult;
					hr = pfd->GetResult(&psiResult);
					if (SUCCEEDED(hr))
					{
						PTSTR pszFilePath = nullptr;
						hr = psiResult->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
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
					pfd->Release();
				}
			}
			else
			{
				PostMessage(hProbCalculator, BPC_DISCONNECT, 0, 0);
				disconnectCalculator();
			}
			return (INT_PTR)TRUE;
		case IDC_L_CONFIRM_BUTTON:
		case IDC_R_CONFIRM_BUTTON:
			calcAimAmount(LOWORD(wParam) - IDC_L_CONFIRM_BUTTON);
			return (INT_PTR)TRUE;
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
			oddsEdits[((LPNMHDR)lParam)->idFrom - IDC_L_BANKER_ODDS_SPIN].spinDelta(((LPNMUPDOWN)lParam)->iDelta);
			Button_SetCheck(hBankerBetSelectors[((LPNMHDR)lParam)->idFrom - IDC_L_BANKER_ODDS_SPIN], 1);
			Button_SetCheck(hBankerBetSelectors[(((LPNMHDR)lParam)->idFrom - IDC_L_BANKER_ODDS_SPIN) ^ 1], 0);
			return (INT_PTR)TRUE;
		case IDC_MOVE_SPIN:
			{
				int itemIdx = betLists[selSide].moveSel(((LPNMUPDOWN)lParam)->iDelta < 0);
				if (itemIdx == -1)
				{
					return (INT_PTR)TRUE;
				}
				if (IsWindowVisible(allBoughtButton.getHwnd()))
				{
					model.swapBanker(selSide, itemIdx);
				}
				else
				{
					model.swapBet(selSide, itemIdx);
				}
				return (INT_PTR)TRUE;
			}
		}
		break;
	case WM_DESTROY:
		if (hProbCalculator != nullptr)
		{
			PostMessage(hProbCalculator, BPC_DISCONNECT, 0, 0);
			DeleteObject(hProbCalculatorIcon);
		}
		break;
	}
	return (INT_PTR)FALSE;
}

void BetTabDlg::updateCurrentProfit()
{
	TCHAR str[20];
	_i64tot(model.getTotalInvest(), str, 10);
	SetWindowText(hTotalInvestText, str);
	_i64tot(model.getProfit(0), str, 10);
	SetWindowText(hCurrentProfitTexts[0], str);
	_i64tot(model.getProfit(1), str, 10);
	SetWindowText(hCurrentProfitTexts[1], str);
	ListBox_ResetContent(resultLists[0].getHwnd());
	updateMinOdds();
}

void BetTabDlg::updateMinOdds()
{
	ListBox_ResetContent(resultLists[1].getHwnd());
	ListBox_ResetContent(resultLists[2].getHwnd());
	if (initialAmount > model.getTotalInvest())
	{
		TCHAR str[14];
		_i64tot(initialAmount - model.getTotalInvest(), str, 10);
		SetWindowText(hRemainingAmountText, str);
		if (winProb > 0)
		{
			double referenceOdds[8]{};
			model.calcReferenceOdds(initialAmount, winProb, winProbError, referenceOdds);
			for (int i = 0; i < 8; i++)
			{
				if (referenceOdds[i] == 0)
				{
					SetWindowText(hReferenceOddsTexts[i], _T("-"));
				}
				else
				{
					_stprintf(str, _T("%.1f"), referenceOdds[i]);
					SetWindowText(hReferenceOddsTexts[i], str);
				}
			}
			return;
		}
	}
	else
	{
		SetWindowText(hRemainingAmountText, _T("N/A"));
	}
	for (int i = 0; i < 8; i++)
	{
		SetWindowText(hReferenceOddsTexts[i], _T("N/A"));
	}
}

void BetTabDlg::add(int side)
{
	if (betLists[side].getBetsSize() + betLists[side].getBankersSize() >= MAX_BET_COUNT)
	{
		return;
	}
	TCHAR str[8];
	amountEdits[side].getText(str, 8);
	int amount = _ttoi(str);
	if (amount == 0)
	{
		amountEdits[side].setSel(0, -1);
		SetFocus(amountEdits[side].getHwnd());
		return;
	}
	if (Button_GetCheck(hBankerBetSelectors[2 * side + 1]))
	{
		Bet& bet = model.addBet(side, oddsEdits[2 * side + 1].getOdds(), amount);
		betLists[side].addBet(bet.show);
	}
	else
	{
		Banker banker(oddsEdits[2 * side].getOdds(), amount);
		model.addBanker(side, banker);
		betLists[side].addBanker(banker.show);
	}
	SetFocus(amountEdits[side].getHwnd());
	updateCurrentProfit();
}

void BetTabDlg::calcBalanceAimAmount()
{
	bool isBet = Button_GetCheck(hBankerBetSelectors[5]);
	TCHAR str[30];
	_stprintf(str, _T("%s %0.1f"), isBet ? _T("下注") : _T("庄家"), oddsEdits[4 + isBet].getOdds());
	if (ListBox_FindString(resultLists[0].getHwnd(), -1, str) == LB_ERR)
	{
		auto result = model.calcAimAmountBalance(isBet, isBet ? oddsEdits[5].getOdds() : oddsEdits[4].getOdds());
		if (result.first <= 0)
		{
			lstrcat(str, _T("     收益已平衡"));
		}
		else
		{
			int i = 6;
			if (result.first < 10000000LL)
			{
				i += _stprintf(&str[i], _T("  %7lld"), result.first);
			}
			else if (result.first < 100000000LL)
			{
				i += _stprintf(&str[i], _T(" %.1f万"), result.first * 1e-4);
			}
			else if (result.first < 1000000000LL)
			{
				i += _stprintf(&str[i], _T("  %lld万"), (long long)round(result.first * 1e-4));
			}
			else
			{
				int decimal = 0;
				for (long long j = 1000000000000LL; j > result.first; decimal++, j /= 10);
				i += _stprintf(&str[i], _T(" %6.*f亿"), decimal, result.first * 1e-8);
			}
			if (result.second < 1000000000LL && result.second > -100000000LL)
			{
				_stprintf(&str[i], _T(" %9lld"), result.second);
			}
			else if (result.second < 100000000000LL && result.second > -10000000000LL)
			{
				_stprintf(&str[i], _T(" %7lld万"), (long long)round(result.second * 1e-4));
			}
			else if (result.second < 10000000000000LL && result.second > -1000000000000LL)
			{
				int decimal = 0;
				for (long long j = 1000000000000LL; j * 10 > result.second && -j < result.second; decimal++, j /= 10);
				_stprintf(&str[i], _T(" %7.*f亿"), decimal, result.second * 1e-8);
			}
		}
		int index = resultLists[0].addString(str, DT_VCENTER);
	}
	ListBox_SelectString(resultLists[0].getHwnd(), -1, str);
	SetFocus(oddsEdits[4 + isBet].getHwnd());
}

void BetTabDlg::updateInitialAmount()
{
	TCHAR str[20];
	initialAmountEdit.getText(str, 20);
	long long newAmount = _ttoll(str);
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
	double newError = _ttoi(str) / 100.0;
	if (newError != winProbError)
	{
		winProbError = newError;
		updateMinOdds();
	}
}

void BetTabDlg::disconnectCalculator()
{
	hProbCalculator = nullptr;
	winProbCalculatorButton.setIcon(hCalculatorIcon);
	DeleteObject(hProbCalculatorIcon);
	hProbCalculatorIcon = nullptr;
	TCHAR winProbCalculatorTipText[] = _T("加载胜率计算器");
	setToolTipText(hWinProbCalculatorTip, winProbCalculatorButton.getHwnd(), hDlg, winProbCalculatorTipText);
}

void BetTabDlg::calcAimAmount(int side)
{
	bool isBet = Button_GetCheck(hBankerBetSelectors[2 * side + 7]);
	if (initialAmount <= model.getTotalInvest())
	{
		if (ListBox_FindString(resultLists[side + 1].getHwnd(), -1, _T("初始数量过低")) == -1)
		{
			resultLists[side + 1].addString(_T("初始数量过低"), DT_VCENTER | DT_CENTER);
		}
		resultLists[side + 1].setCurSel(0);
		initialAmountEdit.setSel(0, -1);
		SetFocus(initialAmountEdit.getHwnd());
		return;
	}
	if (winProb == 0)
	{
		if (ListBox_FindString(resultLists[side + 1].getHwnd(), -1, _T("输入胜率为0")) == -1)
		{
			resultLists[side + 1].addString(_T("输入胜率为0"), DT_VCENTER | DT_CENTER);
		}
		resultLists[side + 1].setCurSel(0);
		winProbEdit.setSel(0, -1);
		SetFocus(winProbEdit.getHwnd());
		return;
	}
	TCHAR str[20];
	_stprintf(str, _T("%s %0.1f"), isBet ? _T("下注") : _T("庄家"), oddsEdits[6 + 2 * side + isBet].getOdds());
	if (ListBox_FindString(resultLists[side + 1].getHwnd(), -1, str) == LB_ERR)
	{
		long long aimAmount = model.calcAimAmountProb(initialAmount, winProb, winProbError, side, isBet, oddsEdits[6 + 2 * side + isBet].getOdds());
		if (aimAmount > initialAmount - model.getTotalInvest())
		{
			lstrcat(&str[6], _T("   全部"));
		}
		else if (aimAmount < 10000000LL)
		{
			_stprintf(&str[6], _T(" %7lld"), aimAmount);
		}
		else if (aimAmount < 1000000000LL)
		{
			_stprintf(&str[6], _T(" %5lld万"), (long long)round(aimAmount * 1e-4));
		}
		else
		{
			int decimal = 0;
			for (long long i = 100000000000LL; i > aimAmount; decimal++, i /= 10);
			_stprintf(&str[6], _T(" %5.*f亿"), decimal, aimAmount * 1e-8);
		}
		resultLists[side + 1].addString(str, DT_VCENTER);
	}
	ListBox_SelectString(resultLists[side + 1].getHwnd(), -1, str);
	SetFocus(oddsEdits[6 + 2 * side + isBet].getHwnd());
}