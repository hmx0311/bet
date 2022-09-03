#pragma once
#include "Edit.h"
class OddsEdit :
	public Edit
{
private:
	double odds = 0.1;
public:
	virtual LRESULT editProc(UINT, WPARAM, LPARAM);
	double getOdds();
	void updateOdds();
	void oddsUp();
	void oddsDown();
};