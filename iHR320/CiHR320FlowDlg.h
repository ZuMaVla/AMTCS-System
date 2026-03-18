#pragma once
#include "afxvslistbox.h"
#include "MyVSListBox.h"
#include "afxcmn.h"
#include "afxbutton.h"
#include "afxwin.h"

class CiHR320Dlg;										// Forward declaration of main window's class

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
	CString m_nextUserAction = _T("Continue");
protected:
	CiHR320Dlg* m_mainWnd;
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
	CMyVSListBoxTS m_ExpFlowLogs;
	CProgressCtrl m_expProgressBar;
	// Button to request repeating last measured temperature
	CMFCButton m_repeatPreviousTBtn;
	afx_msg void OnBnClickedRepeatT();
	// Button to pause/continue experiment
	CMFCButton m_pauseResumeBtn;
	afx_msg void OnBnClickedPause();
	// Buttom to cancel experiment
	CMFCButton m_cancelExp;
	afx_msg void OnClickedCancelExp();
	// Button to add extra temperature data point
	CMFCButton m_addNewTBtn;
	afx_msg void OnClickedAddNewT();
	afx_msg void OnKillfocusNewT();
	CEdit m_newAddT;
	// Stati text for current T target
	CStatic m_currTargetT;
};
