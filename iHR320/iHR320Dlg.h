
// iHR320Dlg.h : header file
//

#pragma once

class CiHR320DlgAutoProxy;
#include <afxcmn.h>
#include "CiHR320ConnectivityDlg.h"
#include "CiHR320SettingsDlg.h"
#include "CiHR320FlowDlg.h"


// CiHR320Dlg dialog
class CiHR320Dlg : public CDialogEx
{
	DECLARE_DYNAMIC(CiHR320Dlg);
	friend class CiHR320DlgAutoProxy;

// Construction
public:
	CiHR320Dlg(CWnd* pParent = NULL);	// standard constructor
	virtual ~CiHR320Dlg();

// Dialog Data
	enum { IDD = IDD_CONNECTIVITY_DLG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	CiHR320DlgAutoProxy* m_pAutoProxy;
	HICON m_hIcon;
	CTabCtrl m_tab;
	CiHR320ConnectivityDlg m_connectivityDlg;
	CiHR320SettingsDlg m_settingsDlg;
	CiHR320FlowDlg m_flowDlg;
	BOOL CanExit();

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnClose();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnTabSelChange(NMHDR* pNMHDR, LRESULT* pResult);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnStnClickedPlcconnectedText2();
	afx_msg void OnStnClickedTcconnectedText2();
};
