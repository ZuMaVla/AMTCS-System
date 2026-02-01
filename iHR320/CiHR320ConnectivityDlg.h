#pragma once
#include "afxvslistbox.h"
#include "afxwin.h"
#include "afxcmn.h"
#include <string>
#include <array>


// CiHR320ConnectivityDlg dialog

class CiHR320ConnectivityDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CiHR320ConnectivityDlg)

public:
	CiHR320ConnectivityDlg(CWnd* pParent = NULL);		// standard constructor
	virtual ~CiHR320ConnectivityDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CONNECTIVITY_DLG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedConnectButton();
	void UpdateSystemStatusUI(std::string device);
	std::array<int, 4> GetIPAddress(std::string type);
	
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
	CVSListBox m_ConnectionLogs;
	CButton m_CheckBoxPLC;
	
	CIPAddressCtrl m_localIP;							// variable for IP on the network with RPi/PLC
	
	CIPAddressCtrl m_instIP;							// IP on the institutional network
};
