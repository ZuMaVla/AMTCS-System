// CiHR320ConnectivityDlg.cpp : implementation file
//

#include "stdafx.h"
#include "iHR320.h"
#include "CiHR320ConnectivityDlg.h"
#include "afxdialogex.h"
#include <string>
#include "TCPtoRPi.h"


// CiHR320ConnectivityDlg dialog

IMPLEMENT_DYNAMIC(CiHR320ConnectivityDlg, CDialogEx)

CiHR320ConnectivityDlg::CiHR320ConnectivityDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_CONNECTIVITY_DLG, pParent)
{

}

CiHR320ConnectivityDlg::~CiHR320ConnectivityDlg()
{
}

void CiHR320ConnectivityDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CONN_LOGS, m_ConnectionLogs);
	DDX_Control(pDX, IDC_CHECK_PLC, m_CheckBoxPLC);
}


BEGIN_MESSAGE_MAP(CiHR320ConnectivityDlg, CDialogEx)
	ON_BN_CLICKED(IDC_CONNECT_BUTTON, &CiHR320ConnectivityDlg::OnBnClickedConnectButton)
END_MESSAGE_MAP()


// CiHR320ConnectivityDlg message handlers

BOOL CiHR320ConnectivityDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	m_ConnectionLogs.EnableBrowseButton(FALSE);
	m_ConnectionLogs.AddItem(_T("Ready to check connectivity..."));



	return TRUE;
}

void CiHR320ConnectivityDlg::OnBnClickedConnectButton()
{
	std::string reply;

	if (SendTCPMessage("192.168.50.1", 5050, "Hello", reply)) {
		m_ConnectionLogs.AddItem(_T("PLC ready on 192.168.50.1"));
		m_CheckBoxPLC.SetCheck(TRUE);
		m_CheckBoxPLC.SetWindowText(_T("Connected"));
	}
	else {
		AfxMessageBox(_T("Connection failed"));
		m_ConnectionLogs.AddItem(_T("PLC offline"));
		m_CheckBoxPLC.SetCheck(FALSE);
		m_CheckBoxPLC.SetWindowText(_T("Offline"));
	}
}
