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
	RECT rcErase1 = { 0 };
	RECT rcErase2 = { 0 };
	long long initialAmount = 0;
	BOOL winProbSide = 0;
	double winProb = 0;
	double winProbError = 0;
	Model model;

	HWND hProbabilityCalculator = nullptr;
	HICON hProbabilityCalculatorIcon = nullptr;

protected:
	Button resetButton;
	HWND hTotalInvestText;
	HWND hCurrentProfitText[2];
	HWND haveClosingCheck;
	BetList betList[2];
	HWND hBankerBetSelector[10];
	OddsEdit oddsEdit[10];
	NumericEdit amountEdit[2];
	Button clearAmountButton[2];
	Button addAmountButton[8];
	Button addButton[2];
	NumericEdit boughtEdit;
	HWND hMoveSpin;
	Button allBoughtButton;
	ListBox resultList[3];
	Button confirmButton[3];
	NumericEdit initialAmountEdit;
	HWND hWinProbSideLeftSelector;
	NumericEdit winProbEdit;
	NumericEdit winProbErrorEdit;
	Button winProbCalculatorButton;
	HWND hWinProbCalculatorTip;
	HWND hReferenceOddsText[8];
public:
	BetTabDlg();
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