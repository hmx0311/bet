#pragma once

#include <string>

void setVCentered(HWND hEdit);
LRESULT CALLBACK editSubclassProc(HWND hEdit, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);

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

public:
	void attach(HWND hEdit, HWND hBankerOddsEdit, HWND hBetOddsEdit, HWND hBankerSelector);
	virtual LRESULT wndProc(UINT msg, WPARAM wParam, LPARAM lParam);
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