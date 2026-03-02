// CiHR320ConnectivityDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "iHR320Dlg.h"
#include "CiHR320ConnectivityDlg.h"
#include "afxdialogex.h"
#include "TCPtoRPi.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>
#include <array>
#include "DataAcquisition.h"


// CiHR320ConnectivityDlg dialog

IMPLEMENT_DYNAMIC(CiHR320ConnectivityDlg, CDialogEx)

CiHR320ConnectivityDlg::CiHR320ConnectivityDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_CONNECTIVITY_DLG, pParent)
	, m_emulation(FALSE)
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
	DDX_Control(pDX, IDC_LOCAL_IP, m_localIP);
	DDX_Control(pDX, IDC_INET_IP, m_instIP);
	DDX_Control(pDX, IDC_COMBO_CCD, m_CCDSelectCtrl);
	DDX_Control(pDX, IDC_COMBO_MONO, m_MonoSelectCtrl);
	DDX_Control(pDX, IDC_CHECK_CCD, m_CheckBoxCCD);
	DDX_Control(pDX, IDC_CHECK_IHR320, m_CheckBoxMono);
	DDX_Control(pDX, IDC_Acq, m_acquisBtnTemp);
	DDX_Control(pDX, IDC_EDIT1, m_gratingTestTemp);
	DDX_Check(pDX, IDC_SDK_EMULATION, m_emulation);
	DDX_Control(pDX, IDC_CHECK_TC, m_CheckBoxTC);
}


BEGIN_MESSAGE_MAP(CiHR320ConnectivityDlg, CDialogEx)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_CONNECT_BUTTON, &CiHR320ConnectivityDlg::OnBnClickedConnectButton)
	ON_BN_CLICKED(IDC_Acq, &CiHR320ConnectivityDlg::OnBnClickedAcq)
	ON_BN_CLICKED(IDC_MOVE_TO_BTN_TEST, &CiHR320ConnectivityDlg::OnBnClickedMoveToBtnTest)
END_MESSAGE_MAP()



// CiHR320ConnectivityDlg message handlers

BOOL CiHR320ConnectivityDlg::OnInitDialog()
{	
	std::array<int, 4> ip;
	CDialogEx::OnInitDialog();

	ip = GetIPAddress("local");
	m_localIP.SetAddress(ip[0], ip[1], ip[2], ip[3]);

	ip = GetIPAddress("Tyndall");
	m_instIP.SetAddress(ip[0], ip[1], ip[2], ip[3]);

	m_ConnectionLogs.EnableBrowseButton(FALSE);
	m_ConnectionLogs.AddItem(_T("Ready to check connectivity..."));


	return TRUE;
}

void CiHR320ConnectivityDlg::OnBnClickedConnectButton()
{

	if (!(SendTCPMessage(GetIPstrFromCtrl(m_localIP), 5051, "REQUEST PLC_STATUS"))) {
		AfxMessageBox(_T("Connection failed"));
		m_ConnectionLogs.AddItem(_T("PLC offline"));
		m_CheckBoxPLC.SetCheck(FALSE);
		m_CheckBoxPLC.SetWindowText(_T("Offline"));
	}
	StartTimer(TIMER_PLC_CHECK, 5);
	std::array<BOOL, 2> l_FlagSDK = m_mainWnd->ConnectMonoAndCCD();
	std::cout << "Result: " << l_FlagSDK[0] << l_FlagSDK[1] << "\n";
	if (l_FlagSDK[0]) {
		m_CheckBoxCCD.SetCheck(TRUE);
		m_CheckBoxCCD.SetWindowText(_T("Connected"));
		m_ConnectionLogs.AddItem(_T("CCD connected"));
	}
	else
	{
		m_CheckBoxCCD.SetCheck(FALSE);
		m_CheckBoxCCD.SetWindowText(_T("Offline"));
		m_ConnectionLogs.AddItem(_T("CCD not found"));
	}

	if (l_FlagSDK[1]) {
		m_CheckBoxMono.SetCheck(TRUE);
		m_CheckBoxMono.SetWindowText(_T("Connected"));
		m_ConnectionLogs.AddItem(_T("Monochromator connected"));
	}
	else
	{
		m_CheckBoxMono.SetCheck(FALSE);
		m_CheckBoxMono.SetWindowText(_T("Offline"));
		m_ConnectionLogs.AddItem(_T("Monochromator not found"));
	}
}

void CiHR320ConnectivityDlg::StartTimer(UINT_PTR nIDEvent, int _sec) {
    SetTimer(nIDEvent, 100 + 1000*_sec, NULL);
}

void CiHR320ConnectivityDlg::StopTimer(UINT_PTR nIDEvent) {
    KillTimer(nIDEvent);
}

void CiHR320ConnectivityDlg::OnTimer(UINT_PTR nIDEvent) {
    if (nIDEvent == TIMER_PLC_CHECK) {
        m_CheckBoxPLC.SetCheck(FALSE);
        m_CheckBoxPLC.SetWindowText(_T("Not Responsive"));
        m_ConnectionLogs.AddItem(_T("PLC Timeout Error"));
        KillTimer(TIMER_PLC_CHECK);
    }
    CDialogEx::OnTimer(nIDEvent);
}


void CiHR320ConnectivityDlg::UpdateSystemStatusUI(std::string device) {
	if (device == "PLC") {
		m_ConnectionLogs.AddItem(_T("PLC ready on 192.168.50.1"));
		m_CheckBoxPLC.SetCheck(TRUE);
		m_CheckBoxPLC.SetWindowText(_T("Connected"));
		StopTimer(TIMER_PLC_CHECK);
	}
	else if (device == "TC_OK") {
		m_ConnectionLogs.AddItem(_T("TC is alive"));
		m_CheckBoxTC.SetCheck(TRUE);
		m_CheckBoxTC.SetWindowText(_T("Alive"));
	}
	else if (device == "TC_OFF") {
		m_ConnectionLogs.AddItem(_T("TC is not responsive"));
		m_CheckBoxTC.SetCheck(FALSE);
		m_CheckBoxTC.SetWindowText(_T("Off"));
	}
	else if (device == "TC_READY") {
		m_ConnectionLogs.AddItem(_T("TC is ready"));
		m_CheckBoxTC.SetCheck(TRUE);
		m_CheckBoxTC.SetWindowText(_T("Ready"));
	}
	return;
}

void CiHR320ConnectivityDlg::SetMainWnd(CiHR320Dlg * main)
{
	m_mainWnd = main;
}


	

std::array<int, 4> CiHR320ConnectivityDlg::GetIPAddress(std::string type)
{
	char hostname[256];
	if (gethostname(hostname, sizeof(hostname)) == SOCKET_ERROR)
		return{ 0, 0, 0, 0 };

	addrinfo hints = {};
	hints.ai_family = AF_INET; // IPv4 only
	hints.ai_socktype = SOCK_STREAM;

	addrinfo* info = nullptr;
	if (getaddrinfo(hostname, nullptr, &hints, &info) != 0)
		return { 0, 0, 0, 0 };

	std::string ip_str;

	for (addrinfo* p = info; p != nullptr; p = p->ai_next)
	{
		sockaddr_in* addr = reinterpret_cast<sockaddr_in*>(p->ai_addr);
		char ip[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &(addr->sin_addr), ip, sizeof(ip));
		ip_str = ip;

		if ((ip_str.find("192.168.137.") == 0) || (ip_str == "127.0.0.1"))		// not interested in "default" IPs (loopback and ICS)
			continue;

		if (type == "local") {													// looking for IP on the network with RPi
			if (ip_str.find("192.168.") == 0) {
				break;
			}
		}
		else {																	// looking for IP on the institutional LAN
			if (ip_str.find("192.168.") != 0) {
				break;
			}
		}
	}

	freeaddrinfo(info);

	std::array<int, 4> result;

	for (int i = 0; i < 3; i++) {
		auto pos = ip_str.find(".");
		result[i] = std::stoi(ip_str.substr(0, pos));				// converting string to integer
		ip_str = ip_str.substr(pos + 1);
	}
	result[3] = std::stoi(ip_str);

	return result;

}




void CiHR320ConnectivityDlg::OnBnClickedAcq()
{
	TakeSpectrum(m_mainWnd);
}



void CiHR320ConnectivityDlg::OnBnClickedMoveToBtnTest()
{
	CString strTarget;
	
	m_gratingTestTemp.GetWindowText(strTarget);

	double posTarget = _ttof(strTarget);

	m_mainWnd->MonoMoveTo(posTarget);
}
