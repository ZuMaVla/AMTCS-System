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
	CiHR320FlowDlg(CWnd* pParent = NULL);				// standard constructor
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
	CMyVSListBoxTS m_ExpFlowLogs;				// Customised box for experiment logs
	CProgressCtrl m_expProgressBar;				// Experiment progress bar
	CMFCButton m_repeatPreviousTBtn;			// Button to request repeating last measured temperature
	afx_msg void OnBnClickedRepeatT();
	CMFCButton m_pauseResumeBtn;				// Button to pause/continue experiment
	afx_msg void OnBnClickedPause();
	CMFCButton m_cancelExp;						// Buttom to cancel experiment
	afx_msg void OnClickedCancelExp();
	CMFCButton m_addNewTBtn;					// Button to add extra temperature data point
	afx_msg void OnClickedAddNewT();
	afx_msg void OnKillfocusNewT();
	CEdit m_newAddT;							// Box for new Temperature entry
	CStatic m_currTargetT;						// Static text for current T target
};
