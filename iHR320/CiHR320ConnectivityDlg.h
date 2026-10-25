#pragma once
#include "afxvslistbox.h"
#include "afxwin.h"
#include "afxcmn.h"
#include <string>
#include <array>
#include "afxbutton.h"
#include "MyVSListBox.h"


class CiHR320Dlg;		// Forward declaration of main window's class

// CiHR320ConnectivityDlg dialog

class CiHR320ConnectivityDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CiHR320ConnectivityDlg)

public:
	CiHR320ConnectivityDlg(CWnd* pParent = NULL);		// constructor with passed main dialog pointer
	virtual ~CiHR320ConnectivityDlg();
	afx_msg void OnBnClickedConnectButton();
	void UpdateSystemStatusUI(std::string device);
	void CheckHardware(bool isExperiment);
	void SetMainWnd(CiHR320Dlg* main);
	std::array<int, 4> GetIPAddress(std::string type);
	CIPAddressCtrl m_localIP;							// variable for IP on the network with RPi/PLC
	std::string CiHR320ConnectivityDlg::GetIPstrFromCtrl(const CIPAddressCtrl& ctrl)
	{
		BYTE b1, b2, b3, b4;
		ctrl.GetAddress(b1, b2, b3, b4);

		std::string ip_str =
			std::to_string(b1) + "." +
			std::to_string(b2) + "." +
			std::to_string(b3) + "." +
			std::to_string(b4);

		return ip_str;
	}
	
// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CONNECTIVITY_DLG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()

	CiHR320Dlg* m_mainWnd;

	CComPtr<IJYMonoReqd> m_jyMono;
	CIPAddressCtrl m_instIP;							// IP on the institutional network
public:
	CMyVSListBoxTS m_ConnectionLogs;
	CButton m_CheckBoxPLC;
	CComboBox m_CCDSelectCtrl;							// List of CCDs installed in the system	 
	CComboBox m_MonoSelectCtrl;							// List of monochromators installed in the system
	CButton m_CheckBoxCCD;
	CButton m_CheckBoxMono;
	CButton m_CheckBoxTC;								// control for TC status
	CMFCButton m_connectBtn;							// Button that checks hardware connectivity
	BOOL m_emulation = FALSE;							// SDK emulation if true
	CButton m_emulationMode;
	afx_msg void ToggleSdkEmulation();
	CButton m_CheckBoxServer;
};
