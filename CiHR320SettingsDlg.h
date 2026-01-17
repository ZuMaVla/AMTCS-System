#pragma once


// CiHR320SettingsDlg dialog

class CiHR320SettingsDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CiHR320SettingsDlg)

public:
	CiHR320SettingsDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CiHR320SettingsDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_EXPERIMENT_SETTINGS_DLG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
