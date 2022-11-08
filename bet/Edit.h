#pragma once

#include "button.h"

#include <string>

void setVCentered(HWND hEdit);

class Edit
{
protected:
	HWND hEdit;
public:
	void attach(HWND hEdit);
	virtual LRESULT wndProc(UINT msg, WPARAM wParam, LPARAM lParam) = 0;
	HWND getHwnd();
	void setText(PCTSTR str);
	void getText(PTSTR str, int nMaxCount);
	void setSel(int start, int end);
};

class NumericEdit :
	public Edit
{
private:
	std::wstring curUndo = _T("");
	std::wstring lastUndo = _T("");

public:
	virtual LRESULT wndProc(UINT msg, WPARAM wParam, LPARAM lParam);
	void setText(PCTSTR str);
	void resetUndo();
private:
	void updateStr();
};

class AmountEdit :
	public NumericEdit
{
private:
	HWND hBankerOddsEdit;
	HWND hBetOddsEdit;
	HWND hBankerSelector;
	Button clearButton;

public:
	void attach(HWND hEdit, HWND hBankerOddsEdit, HWND hBetOddsEdit, HWND hBankerSelector);
	virtual LRESULT wndProc(UINT msg, WPARAM wParam, LPARAM lParam);
private:
	void setClearButtonPos();
};

class OddsEdit :
	public Edit
{
private:
	double odds = 0.1;

public:
	virtual LRESULT wndProc(UINT msg, WPARAM wParam, LPARAM lParam);
	double getOdds();
	void spinDelta(int iDelta);
private:
	void oddsUp(double up = 0.1);
	void oddsDown(double down = 0.1);
	void updateOdds();
};