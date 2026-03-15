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
	int previousT = 0;
	for (int t : Ts)
	{
		if (t != previousT) {
			CString s;
			s.Format(_T("%d"), t);
			AddItem(s, t);										// reinserting sorted Ts
		}
		previousT = t;
	}
}


int CMyVSListBox::AddItem(const CString& strText, DWORD_PTR dwData, int iIndex)
{
	CString T_str;
	if (strText.IsEmpty())
		m_newT.GetWindowTextW(T_str);
	else
		T_str = strText;
	int value = _ttoi(T_str);
	T_str.Format(_T("%d"), value);
	int index = CVSListBox::AddItem(T_str, value, iIndex);
	return index;
}

int CMyVSListBox::getFirst()
{
	return GetItemData(0);
}

int CMyVSListBox::getLast()
{
	return GetItemData(GetCount() - 1);
}

void CMyVSListBox::AddAll(std::vector<std::string> listTs)
{
	CString s;

	for (const auto& T : listTs)
	{
		s = T.c_str();
		AddItem(s);
	}
}

void CMyVSListBox::RemoveAll()
{
	int count = GetCount();
	for (int i = count - 1; i >= 0; --i)
	{
		RemoveItem(i);
	}

}

std::vector<std::string> CMyVSListBox::GetAllItemTs()
{
	std::vector<std::string> Ts;
	int tCount = GetCount();

	for (int i = 0; i < tCount; i++)
	{
		DWORD_PTR data = GetItemData(i);
		Ts.push_back(std::to_string((int)data));
	}
	return Ts;
}

void CMyVSListBox::OnSelectionChanged()
{
	CVSListBox::OnSelectionChanged();
}

BOOL CMyVSListBox::EditItem(int iItem)
{
	return FALSE;
}

int CMyVSListBoxTS::AddItem(const CString & strText)
{
	CTime currentTime = CTime::GetCurrentTime();
	CString l_timeStamp = currentTime.Format(_T("[%Y/%m/%d %H:%M:%S] "));
	int index = CVSListBox::AddItem(l_timeStamp + strText);
	SelectItem(index);
	return index;
}

void CMyVSListBoxTS::RemoveAll()
{
	int count = GetCount();
	for (int i = count - 1; i >= 0; --i)
	{
		RemoveItem(i);
	}
}
