#pragma once

#include "afxvslistbox.h"

class CMyVSListBox : public CVSListBox
{
public:
	CMyVSListBox() {}
	virtual ~CMyVSListBox() {}
	CEdit m_newT;										// New temperature to be added to the list of temperatures 
	CSpinButtonCtrl m_mod_new_T;						// Spin button for in-/decremental change of the new temperature
	virtual void SortT(BOOL isAscending);
	virtual int AddItem(const CString &strIext, DWORD_PTR  dwData = 0UL, int iIndex = -1);

protected:
	virtual void OnSelectionChanged();
	virtual BOOL EditItem(int iItem);
};
