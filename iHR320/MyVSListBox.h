#pragma once

#include "afxvslistbox.h"
#include <vector>
#include <string>

class CMyVSListBox : public CVSListBox					// For the list of temperatures
{
public:
	CMyVSListBox() {}
	virtual ~CMyVSListBox() {}
	CEdit m_newT;										// New temperature to be added to the list of temperatures 
	bool m_isHT = false;								// Flag of high temperature entries
	CSpinButtonCtrl m_mod_new_T;						// Spin button for in-/decremental change of the new temperature
	virtual void SortT(BOOL isAscending);
	virtual int AddItem(const CString &strText, DWORD_PTR  dwData = 0UL, int iIndex = -1);
	int getFirst();
	int getLast();
	void AddAll(std::vector<std::string> listTs);
	void RemoveAll();
	std::vector<std::string> GetAllItemTs();
protected:
	virtual void OnSelectionChanged();
	virtual BOOL EditItem(int iItem);
};



class CMyVSListBoxTS : public CVSListBox				// For logs (with time stamps)
{
public:
	CMyVSListBoxTS() {}
	virtual ~CMyVSListBoxTS() {}
	virtual int AddItem(const CString &strText);
	void RemoveAll();
protected:
};
