#pragma once
#include "afxvslistbox.h"


// CiHR320FlowDlg dialog

class CiHR320FlowDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CiHR320FlowDlg)

public:
	CiHR320FlowDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CiHR320FlowDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_EXPERIMENT_FLOW_DLG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
	CVSListBox m_ExpFLowLogs;
};
