#pragma once
#include "Edit.h"

#include "string"
class NumericEdit :
    public Edit
{
private:
    std::wstring curUndo = _T("");
    std::wstring lastUndo = _T("");
private:
    void updateStr();
public:
    virtual LRESULT editProc(UINT, WPARAM, LPARAM);
    void setText(LPCWSTR str);
};