
// iHR320Dlg.h : header file
//

#pragma once

class CiHR320DlgAutoProxy;


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
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_IHR320_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	CiHR320DlgAutoProxy* m_pAutoProxy;
	HICON m_hIcon;

	BOOL CanExit();

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnClose();
	virtual void OnOK();
	virtual void OnCancel();
	DECLARE_MESSAGE_MAP()
};
