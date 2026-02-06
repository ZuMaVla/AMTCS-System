#include "stdafx.h"
#include "MyVSListBox.h"
#include <string>
#include "afxwin.h"
#include <vector>
#include <algorithm>
#include "DefaultExperimentSettings.h"



void CMyVSListBox::SortT(BOOL isAscending)
{
	 
	int count = GetCount();									// amount of Ts in the list
	std::vector<int> Ts;
	Ts.reserve(count);
	for (int i = 0; i < count; i++)
	{
		Ts.push_back(static_cast<int>(GetItemData(i)));		// Ts from Item data
	}
	std::sort(Ts.begin(), Ts.end());						// sorting ascending
	if (!isAscending) {
		std::reverse(Ts.begin(), Ts.end());					// sorting descending (reversing)
	}
	for (int i = count - 1; i >= 0; i--)
		RemoveItem(i);										// removing all Ts before inserting sorted ones
	for (int t : Ts)
	{
		CString s;
		s.Format(_T("%d"), t);
		AddItem(s, t);										// reinserting sorted Ts
	}
}


int CMyVSListBox::AddItem(const CString &strIext, DWORD_PTR  dwData, int iIndex)
{
	CString T_str;
	if (strIext.IsEmpty()){
		m_newT.GetWindowTextW(T_str);
	}
	else {
		T_str = strIext;
	}
	dwData = _ttoi(T_str);
	T_str.Format(_T("%d"), static_cast<int>(dwData));
	int index = CVSListBox::AddItem(T_str, dwData, iIndex);

	return index;
}


void CMyVSListBox::OnSelectionChanged()
{
	CVSListBox::OnSelectionChanged();
}

BOOL CMyVSListBox::EditItem(int iItem)
{
	return FALSE;
}

