
// iHR320Dlg.cpp : implementation file
//

#include "stdafx.h"
#include "iHR320.h"
#include "iHR320Dlg.h"
#include "DlgProxy.h"
#include "afxdialogex.h"
#include "Resource.h"
#include "CiHR320ConnectivityDlg.h"
#include "TCPtoRPi.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CiHR320Dlg dialog


IMPLEMENT_DYNAMIC(CiHR320Dlg, CDialogEx);

CiHR320Dlg::CiHR320Dlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_IHR320_MAIN_DLG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_pAutoProxy = NULL;
}

CiHR320Dlg::~CiHR320Dlg()
{
	// If there is an automation proxy for this dialog, set
	//  its back pointer to this dialog to NULL, so it knows
	//  the dialog has been deleted.
	if (m_pAutoProxy != NULL)
		m_pAutoProxy->m_pDialog = NULL;
}

void CiHR320Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CiHR320Dlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_CLOSE()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_MESSAGE(WM_UPDATE_SYSTEM_STATUS, &CiHR320Dlg::OnUpdateSystemStatus)
	ON_STN_CLICKED(IDC_TC_connected_Text2, &CiHR320Dlg::OnStnClickedTcconnectedText2)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_MAIN, &CiHR320Dlg::OnTabSelChange)
END_MESSAGE_MAP()


// CiHR320Dlg message handlers

BOOL CiHR320Dlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_tab.SubclassDlgItem(IDC_TAB_MAIN, this);

	m_tab.InsertItem(0, _T("Connectivity"));
	m_tab.InsertItem(1, _T("Experiment Settings"));
	m_tab.InsertItem(2, _T("Experiment Flow"));

	// Create child dialogs
	m_connectivityDlg.Create(IDD_CONNECTIVITY_DLG, &m_tab);
	m_settingsDlg.Create(IDD_EXPERIMENT_SETTINGS_DLG, &m_tab);
	m_flowDlg.Create(IDD_EXPERIMENT_FLOW_DLG, &m_tab);

	CRect rcTab;
	m_tab.GetClientRect(&rcTab);
	m_tab.AdjustRect(FALSE, &rcTab);   // remove tab headers

	m_tab.SetCurSel(0);

	m_connectivityDlg.MoveWindow(&rcTab);
	m_connectivityDlg.ShowWindow(SW_SHOW);

	// create other tabs, hide them initially
	m_settingsDlg.MoveWindow(&rcTab);
	m_settingsDlg.ShowWindow(SW_HIDE);

	m_flowDlg.MoveWindow(&rcTab);
	m_flowDlg.ShowWindow(SW_HIDE);
	// Add "About..." menu item to system menu.



	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here

	StartMainLogicThread(this);		// Main communication-with-PLC logic gets access to the UI (this)


	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CiHR320Dlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CiHR320Dlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CiHR320Dlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

// Automation servers should not exit when a user closes the UI
//  if a controller still holds on to one of its objects.  These
//  message handlers make sure that if the proxy is still in use,
//  then the UI is hidden but the dialog remains around if it
//  is dismissed.

void CiHR320Dlg::OnClose()
{
	m_askUser.s_question = L"Are you sure you want to exit?";
	isExitEnabled = m_askUser.DoModal() == IDOK;
	if (CanExit())
		CDialogEx::OnClose();
}

void CiHR320Dlg::OnOK()
{
	if (CanExit())
		CDialogEx::OnOK();
}

void CiHR320Dlg::OnCancel()
{
	if (CanExit())
		CDialogEx::OnCancel();
}

BOOL CiHR320Dlg::CanExit()
{
	return isExitEnabled;
}



void CiHR320Dlg::OnStnClickedPlcconnectedText2()
{
	// TODO: Add your control notification handler code here
}


void CiHR320Dlg::OnStnClickedTcconnectedText2()
{
	// TODO: Add your control notification handler code here
}

void CiHR320Dlg::OnTabSelChange(NMHDR* pNMHDR, LRESULT* pResult)
{
	int sel = m_tab.GetCurSel();

	// Hide all pages
	m_flowDlg.ShowWindow(SW_HIDE);
	m_connectivityDlg.ShowWindow(SW_HIDE);
	m_settingsDlg.ShowWindow(SW_HIDE);

	// Show the selected page
	switch (sel)
	{
	case 0: m_connectivityDlg.ShowWindow(SW_SHOW); break;
	case 1: m_settingsDlg.ShowWindow(SW_SHOW); break;
	case 2: m_flowDlg.ShowWindow(SW_SHOW); break;
	}

	*pResult = 0;
}

std::string CiHR320Dlg::GetLocalIP() {
	return m_connectivityDlg.GetIPstrFromCtrl(m_connectivityDlg.m_localIP);
}


LRESULT CiHR320Dlg::OnUpdateSystemStatus(WPARAM wParam, LPARAM lParam)
{
	std::string* device = reinterpret_cast<std::string*>(lParam);

	m_connectivityDlg.UpdateSystemStatusUI(*device);

	delete device;   // free the memory after using
	return 0;
}
