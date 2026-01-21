#pragma once


// CiHR320ConnectivityDlg dialog

class CiHR320ConnectivityDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CiHR320ConnectivityDlg)

public:
	CiHR320ConnectivityDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CiHR320ConnectivityDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CONNECTIVITY_DLG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
