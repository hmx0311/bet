#pragma once
#include "Button.h"
class CheckBox :
    public Button
{
private:
	CHECKBOXSTATES lastState = CBS_UNCHECKEDNORMAL;
    bool check;
public:
	virtual void drawItem(HDC hDC, UINT itemState, RECT& rcItem);
	void setCheck(bool check);
	bool getCheck();
private:
	void drawCheckBox(HDC hDC, CHECKBOXSTATES state, RECT& rcItem);
};

