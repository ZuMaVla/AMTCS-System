#pragma once
#include "afxvslistbox.h"
#include "MyVSListBox.h"
#include "afxcmn.h"

class CiHR320Dlg;
// CiHR320FlowDlg dialog

class CiHR320FlowDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CiHR320FlowDlg)

public:
	CiHR320FlowDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CiHR320FlowDlg();
	void SetMainWnd(CiHR320Dlg* main);
// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_EXPERIMENT_FLOW_DLG };
#endif

protected:
	CiHR320Dlg* m_mainWnd;
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
	CMyVSListBoxTS m_ExpFLowLogs;
	CProgressCtrl m_expProgressBar;
};
