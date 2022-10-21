#pragma once
#include "Dialog.h"

#include "button.h"
#include "edit.h"
#include "listbox.h"
#include "model.h"

class BetTabDlg :
	public Dialog
{
public:
	static HICON hResetIcon;
	static HICON hClearIcon;
	static HICON hTickIcon;
	static HICON hCalculatorIcon;
private:
	bool selSide = 0;
	long long initialAmount = 0;
	bool winProbSide = 0;
	double winProb = 0;
	double winProbError = 0;
	Model model;

	HWND hProbCalculator = nullptr;
	HICON hProbCalculatorIcon = nullptr;

protected:
	Button resetButton;
	HWND hTotalInvestText;
	HWND hCurrentProfitTexts[2];
	HWND hHaveClosingCheck;
	BetList betLists[2];
	HWND hBankerBetSelectors[10];
	OddsEdit oddsEdits[10];
	AmountEdit amountEdits[2];
	Button addAmountButtons[8];
	Button addButtons[2];
	NumericEdit boughtEdit;
	Button allBoughtButton;
	ListBox resultLists[3];
	Button calculateButtons[3];
	NumericEdit initialAmountEdit;
	HWND hRemainingAmountText;
	HWND hWinProbSideLeftSelector;
	NumericEdit winProbEdit;
	NumericEdit winProbErrorEdit;
	Button winProbCalculatorButton;
	HWND hWinProbCalculatorTip;
	HWND hReferenceOddsTexts[8];
public:
	BetTabDlg(double cut);
	virtual INT_PTR initDlg(HWND hDlg);
	virtual INT_PTR dlgProc(UINT msg, WPARAM wParam, LPARAM lParam);
private:
	void updateCurrentProfit();
	void updateMinOdds();
	void add(int uid);
	void calcBalanceAimAmount();
	void updateInitialAmount();
	void updateWinProb();
	void updateWinProbError();
	void disconnectCalculator();
	void calcAimAmount(int side);
};