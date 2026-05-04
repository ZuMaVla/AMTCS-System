
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
#include "JYDeviceSink.h"
#include <string>
#include <iostream>
#include "Security.h"



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
	m_jyMono = NULL;
	m_jyCCD = NULL;
	m_pConfigBrowser = NULL;
	m_isMonoInitialised = FALSE;
	m_bMeasurementStarted = FALSE;
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
	ON_WM_TIMER()
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_MAIN, &CiHR320Dlg::OnTabSelChange)
	ON_MESSAGE(WM_UPDATE_SYSTEM_EVENT, &CiHR320Dlg::OnUpdateSystemEvent)
	ON_MESSAGE(WM_UPDATE_SYSTEM_STATUS, &CiHR320Dlg::OnUpdateSystemStatus)
	ON_MESSAGE(WM_USER_MONO_LOG_MESSAGE, &CiHR320Dlg::OnMonoLogMessage)
	ON_MESSAGE(WM_USER_LOG_MESSAGE, &CiHR320Dlg::OnPutLog)
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
	m_connectivityDlg.SetMainWnd(this);
	m_connectivityDlg.Create(IDD_CONNECTIVITY_DLG, &m_tab);
	m_settingsDlg.SetMainWnd(this);
	m_settingsDlg.Create(IDD_EXPERIMENT_SETTINGS_DLG, &m_tab);
	m_flowDlg.SetMainWnd(this);
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
	EnableDlg(&m_settingsDlg, FALSE);

	m_flowDlg.MoveWindow(&rcTab);
	m_flowDlg.ShowWindow(SW_HIDE);
	EnableDlg(&m_flowDlg, FALSE);
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

//********************************---SDK initialisation---***************************************

	HRESULT hr = CoCreateInstance(
		__uuidof(JYConfigBrowerInterface),
		NULL, CLSCTX_INPROC_SERVER,
		__uuidof(IJYConfigBrowerInterface),
		(void**)&m_pConfigBrowser
	);
	m_pConfigBrowser->Load();

	LoadMonos();
	LoadCCDs();
	UpdateData(true);


//********************************---Logic+TCP-listener---***************************************

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
	if (!m_isNextExpRefused) {
		m_askUser.s_question = L"Are you sure you want to exit?";
		isExitEnabled = m_askUser.DoModal() == IDOK;
	}
	else {
		isExitEnabled = TRUE;
	}

	if (CanExit()) {
		if (m_connectivityDlg.m_CheckBoxPLC.GetCheck()) 
		{
			SendTCPMessage(this, m_connectivityDlg.GetIPstrFromCtrl(m_connectivityDlg.m_localIP), 5051, "EVENT __STOP__");
			while (!m_isPLCConfirmedOff)
			{
				MSG msg;
				while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
				{
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
				Sleep(50);
			}
		}
		CDialogEx::OnClose();
		CDialogEx::OnCancel();
	}
		
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

	if (pResult)
		*pResult = 0;
}

void CiHR320Dlg::SelectTab(int index)
{
	m_tab.SetCurSel(index);
	OnTabSelChange(nullptr, nullptr);
}

std::string CiHR320Dlg::GetLocalIP() {
	return m_connectivityDlg.GetIPstrFromCtrl(m_connectivityDlg.m_localIP);
}

CString CiHR320Dlg::GetCurrentDir()
{
	CString path;
	m_settingsDlg.m_workDir.GetWindowTextW(path);
	return path;
}

std::array<double, 5> CiHR320Dlg::GetCentresWL(int startWL, int DGRangeNo)
{
	std::array<double, 5> centres = { -1, -1, -1, -1, -1 };
	double _startWL = 0.0;
	double _endWL = 0.0;
	double errorWL, currentStartWL, approxSpectrWin, centreWL, step;
	approxSpectrWin = 80000/m_settingsDlg.m_ListBoxDG.GetItemData(m_settingsDlg.m_ListBoxDG.GetCurSel());
	currentStartWL = startWL;

	for (int i = 0; i < DGRangeNo; i++) {
		centreWL = currentStartWL + approxSpectrWin/2;
		m_jyCCD->GetWavelengthCoverage(centreWL, eUnits, &_startWL, &_endWL);
		errorWL = currentStartWL - _startWL;
		while (abs(errorWL) > 0.01) {
			centreWL += errorWL;
			m_jyCCD->GetWavelengthCoverage(centreWL, eUnits, &_startWL, &_endWL);
			errorWL = currentStartWL - _startWL;
		}
		step = (_endWL - _startWL)/1023;
		centres[i] = centreWL;
		currentStartWL = _endWL + step;
	}
	return centres;
}

LRESULT CiHR320Dlg::OnUpdateSystemStatus(WPARAM wParam, LPARAM lParam)
{
	std::string* device = reinterpret_cast<std::string*>(lParam);

	m_connectivityDlg.UpdateSystemStatusUI(*device);

	delete device;   // free the memory after using
	return 0;
}

LRESULT CiHR320Dlg::OnUpdateSystemEvent(WPARAM wParam, LPARAM lParam)
{
	CString* pEvent = reinterpret_cast<CString*>(lParam);
	CString event = *pEvent;
	if (event == _T("RECOVER_EXP")) {
		m_connectivityDlg.CheckHardware(true);
	}
	else if (event == _T("CONTINUE_EXP")) {
		m_settingsDlg.OnBnClickedStart();
	}
	delete pEvent;   // free the memory after using
	return 0;
}

LRESULT CiHR320Dlg::OnPutLog(WPARAM wParam, LPARAM lParam)
{
	CString* pStr = reinterpret_cast<CString*>(lParam);
	if (pStr)
	{
		CString msg = *pStr;
		CString log;
		if (msg.Left(3) == _T("EDA")) {				// Experiment details accepted
			log = _T("[PLC] ") + msg.Mid(5);
			m_flowDlg.m_ExpFlowLogs.AddItem(log);
			EnableExpFlowDlg();
			SetExpProgress();
			m_isExperimentStarted = TRUE;
			StopTimer(TIMER_EXP_SENT);
			SelectTab(2);
		}
		else if (msg.Left(3) == _T("EDF")) {		// Experiment details failed
			log = _T("[PLC] ") + msg.Mid(5);
			AfxMessageBox(log);
			m_flowDlg.m_ExpFlowLogs.AddItem(log);
			EnableExpSettDlg();
			m_isExperimentStarted = FALSE;
			StopTimer(TIMER_EXP_SENT);
		}
		else if (msg.Left(2) == _T("T=")) {			// Temperature received at init
			log = _T("[TC] Current temperature received: ") + msg.Mid(2) + _T(" K");
			m_currT = _tstof(msg.Mid(2));
			m_connectivityDlg.m_ConnectionLogs.AddItem(log);
		}
		else if (msg.Left(9) == _T("TARGET_T=")) {	// New temperature target
			log = _T("[TC] New target set: ") + msg.Mid(9) + _T(" K");
			m_flowDlg.m_ExpFlowLogs.AddItem(log);
			log = _T("Current target => ") + msg.Mid(9) + _T(" K");
			m_flowDlg.m_currTargetT.SetWindowTextW(log);
		}
		else if (msg.Left(3) == _T("CT=")) {		// Temperature at request of spectrum
			log = _T("[TC] Target temperature reached. Current T: ") + msg.Mid(4) + _T(" K");
			m_currT = _tstof(msg.Mid(4));
			m_flowDlg.m_ExpFlowLogs.AddItem(log);
			EnableDlg(&m_flowDlg, FALSE);
		}
		else if (msg.Left(3) == _T("SAd")) {		// Notification about spectrum acquired
			log = _T("[iHR320] Spectrum acquisition completed; data saved.");

			m_flowDlg.m_ExpFlowLogs.AddItem(log);
			m_settingsDlg.experimentState.experimentProgressIndex++;
			EnableDlg(&m_flowDlg, TRUE);
			SetExpProgress();
		}
		else if (msg == _T("RESET_PLC")) {			// Request to restart PLC 
			RestartPLC();
		}
		else if (msg == _T("PLC_OK")) {				// Flag PLC is on (again)
			m_flowDlg.m_ExpFlowLogs.AddItem(_T("[PLC] Ready on 192.168.50.1"));
			m_connectivityDlg.m_CheckBoxPLC.SetCheck(TRUE);
			m_connectivityDlg.m_CheckBoxPLC.SetWindowText(_T("Connected"));
		}
		else if (msg == _T("SRV_ON")) {				// Flag Server is online
			m_connectivityDlg.m_ConnectionLogs.AddItem(_T("[server] Status: online"));
			m_connectivityDlg.m_CheckBoxServer.SetCheck(TRUE);
			m_connectivityDlg.m_CheckBoxServer.SetWindowText(_T("Online"));
		}
		else if (msg == _T("SRV_OFF")) {			// Flag Server is offline
			m_connectivityDlg.m_ConnectionLogs.AddItem(_T("[server] Status: offline"));
			m_connectivityDlg.m_CheckBoxServer.SetCheck(FALSE);
			m_connectivityDlg.m_CheckBoxServer.SetWindowText(_T("Offline"));
		}
		else if (msg == _T("EXP_NONE")) {			// No running experiment on PLC
			m_connectivityDlg.m_ConnectionLogs.AddItem(_T("[PLC] No active experiment. Ready to accept one."));
			Sleep(2000);
			SelectTab(1);
		}
		else if (msg == _T("EXP_ON")) {				// Experiment state from PLC received
			m_connectivityDlg.m_ConnectionLogs.AddItem(_T("[PLC] An experiment is running. Sharing details..."));
			std::cout << "RAW JSON RECEIVED (size=" << jsonState.size() << "):\n";
			for (unsigned char c : jsonState) {
				if (std::isprint(c))
					std::cout << c;
				else
					std::cout << "\\x" << std::hex << (int)c << std::dec;
			}
			std::cout << "\n";
			m_settingsDlg.experimentState.importJSONString(jsonState);
			m_settingsDlg.experimentState.deserialiseState();
			m_settingsDlg.SetExperimentParameters();
			m_connectivityDlg.m_ConnectionLogs.AddItem(_T("[UI-APP] Experiment restored. Switch to Experiment flow tab."));
			m_isExperimentStarted = TRUE;
			DisableExpSettDlg();
			DisableConnDlg();
			EnableExpFlowDlg();
			SetThreadExecutionState(ES_CONTINUOUS | ES_SYSTEM_REQUIRED);				// prevent system from sleeping
			m_flowDlg.m_pauseResumeBtn.SetWindowTextW(_T("Continue"));
			Sleep(2000);
			SelectTab(2);
			m_flowDlg.m_ExpFlowLogs.AddItem(_T("[UI-APP] Experiment successfully restored and currently paused."));
		}
		else if (msg == _T("EXP_END")) {				// Message received at the end of experiment
			log = _T("[PLC] Experiment finished. Turn off equipment or start a new experiment.");
			m_flowDlg.m_ExpFlowLogs.AddItem(log);

			m_settingsDlg.experimentState.experimentProgressIndex = -1;
			m_isExperimentStarted = FALSE;

			m_askUser.s_question = _T("Your experiment has finished.\nWould you like to start a new experiment?");
			bool isNewExp = m_askUser.DoModal() == IDOK;

			if (isNewExp) {
				m_flowDlg.m_ExpFlowLogs.RemoveAll();
				SetExpProgress();
				EnableExpSettDlg();						// enable Experimental Setting Dialog if everything is connected
				DisableConnDlg();						// ... and disable Connectivity Dialog (not necessary anymore)
				SelectTab(1);							// Swith to Experiment settings Tab
			}
			else {
				m_isNextExpRefused = TRUE;
				OnClose();
			}
			SetThreadExecutionState(ES_CONTINUOUS);		// normal system behaviour restored
		}


		delete pStr;
	}
	return 0;
}

BOOL CiHR320Dlg::ConnectAndInitCCD()					// Based on SDK		
{
	HRESULT hr = S_OK;

	int nSel = m_connectivityDlg.m_CCDSelectCtrl.GetCurSel();
	std::string *selectedCCD = NULL;
	selectedCCD = (std::string *)m_connectivityDlg.m_CCDSelectCtrl.GetItemDataPtr(nSel);

	m_jyCCD->put_Uniqueid((CComBSTR)selectedCCD->c_str());
	m_jyCCD->Load();
	hr = m_jyCCD->OpenCommunications();
	if (FAILED(hr))
	{
		MessageBox(L"Check Hardware and Try Again...");
		return FALSE;
	}
	
	Sleep(2000);
	m_connectivityDlg.m_ConnectionLogs.AddItem(_T("Initialising CCD...Please wait"));
	UpdateData();
	hr = m_jyCCD->Initialize((CComVariant)false, (CComVariant)m_connectivityDlg.m_emulation);
	SetCCDParams();
	if (FAILED(hr))
	{
		m_connectivityDlg.m_ConnectionLogs.AddItem(_T("Check CCD Camera and Try Again..."));
		return FALSE;
	}
	return TRUE;
}

BOOL CiHR320Dlg::ConnectAndInitMono()					// Based on SDK
{
	HRESULT hr = S_OK;
	if (!m_isMonoInitialised) {
		// Get the user selected id...
		if (m_connectivityDlg.m_CCDSelectCtrl.GetCount())
		{

			CString selectedMono;
			int nSel = m_connectivityDlg.m_MonoSelectCtrl.GetCurSel();
			if (nSel < 10)
			{
				selectedMono = m_monoArray[nSel][0];
			}

			if (selectedMono) {
				// Set the unique id to the instance of the object we created
				hr = m_jyMono->put_Uniqueid((CComBSTR)selectedMono);
				// Tell the device to Load it's configuration
				hr = m_jyMono->Load();
				// Attempt to establish communications with the device.  The
				// communication parameters specified in the device configuration 
				// will be used.   If we fail to find the device, we give the user
				// the ability to select hardware emulation.
				hr = m_jyMono->OpenCommunications();
			}

			if (FAILED(hr))
			{
				MessageBox(L"Check Hardware and Try Again...");
			}
			else
			{
				// Attempt to initialize the device with the appropriate parameters
				UpdateData();
				if (!FAILED(m_jyMono->Initialize((CComVariant)m_isMonoInitialised, (CComVariant)m_connectivityDlg.m_emulation)))
				{
					m_isMonoInitialised = true;
				}
			}
		}
	}
	return m_isMonoInitialised;
}

void CiHR320Dlg::SetCCDParams()						// Based on SDK
{
	UpdateData();
	CString text;
	m_settingsDlg.m_maxAT.GetWindowTextW(text);
	double AT = _ttoi(text)/1000.0;					// Define the integration Time (= max acquisition time) in seconds
	text.Format(_T("%.5f"), AT);
	m_jyCCD->put_IntegrationTime(AT);
	// Select the gain
	m_jyCCD->put_Gain(m_gainCCD[0]);
	// select the adc
	m_jyCCD->SelectADC((jyADCType)m_ADCCCD[0]);
	// Define the Acq Format and number of areas
	jyCCDDataType acqFormat = JYMCD_ACQ_FORMAT_SCAN;

	m_jyCCD->DefineAcquisitionFormat(acqFormat, 1); // only defining 1 area
													// Define each area
													// Define each area (note that you need to provide the size of the area, not the end point)
	long xSize = 1024;
	long ySize = 256;
	m_jyCCD->DefineArea(1, 1, 1, xSize, ySize, 1, ySize);
	long dataSize;
	m_jyCCD->get_DataSize(&dataSize);
	
	VARIANT_BOOL ready = VARIANT_FALSE;
	m_jyCCD->get_ReadyForAcquisition(&ready);		// Confirm that the system is ready for acquisition
	if (!ready)
		MessageBox(L"Controller NOT ready for Acquisition.\nCheck Parameters and try again", MB_OK);
	else
	{
		m_settingsDlg.m_acquisBtnTemp.EnableWindow();
	}
}


std::array<BOOL, 2> CiHR320Dlg::ConnectMonoAndCCD()
{
	std::array<BOOL, 2> result = { FALSE, FALSE };

	result[0] = ConnectAndInitCCD();
	result[1] = ConnectAndInitMono();
	return result;
}

void CiHR320Dlg::LoadMonos()						// Based on SDK		
{
	USES_CONVERSION;

	CComBSTR name, monoID;
	CString strName, strID;
	int i = 0;
	m_pConfigBrowser->GetFirstMono(&name, &monoID);
	strName = W2A(name);
	strID = W2A(monoID);
	m_monoArray[i][0] = strID;
	m_monoArray[i][1] = strName;

	if (strName.IsEmpty() && strID.IsEmpty())
	{
		MessageBox(L"No monos found");
		strName = L"TestMono";
		strID = L"1";
		m_monoArray[i][0] = strID;
		m_monoArray[i][1] = strName;

//		return;

	}
	m_connectivityDlg.m_MonoSelectCtrl.InsertString(i, strName);
	i++;

	while (true)
	{
		m_pConfigBrowser->GetNextMono(&name, &monoID);
		strName = W2A(name);
		strID = W2A(monoID);

		if (strName.IsEmpty() && strID.IsEmpty())
			break;

		m_monoArray[i][0] = strID;
		m_monoArray[i][1] = strName;
		m_connectivityDlg.m_MonoSelectCtrl.InsertString(i, strName);
		i++;
	}

	m_connectivityDlg.m_MonoSelectCtrl.SetCurSel(0);

	HRESULT hr = S_OK;
	CLSID   clsid;

	if (m_jyMono == NULL)
	{
		hr = CLSIDFromProgID(L"JYMono.Monochromator", &clsid);
		std::cout << FAILED(hr) << "\n";
		hr = m_jyMono.CoCreateInstance(clsid, NULL, CLSCTX_ALL);
		std::cout << FAILED(hr) << "\n";

		if (FAILED(hr))
			{
			TRACE("Failed to create Mono Object. Err: %ld", hr);
			return;
		}
		// Create a "Sink" for this object.  This provides a facility for 
		// handling events fired by the mono object.  This class can be extracted and 
		// modified for use in your own code by modifying the constructor to take your Dialog
		// as the input parameter and provide the appropriate callbacks in your class. 
		// See the CJYDeviceSink class for more information.
		m_sinkPtrMono = new CJYDeviceSink(this, m_jyMono);
	}


}

void CiHR320Dlg::LoadCCDs()						// Based on SDK	
{
	USES_CONVERSION;

	CComBSTR name, ccdID;
	CString strName, strccdID;

	m_pConfigBrowser->GetFirstCCD(&name, &ccdID);
	strName = W2A(name);
	strccdID = W2A(ccdID);

	if (strName.IsEmpty() && strccdID.IsEmpty())
	{
		MessageBox(L"No CCDs found");
		strName = L"TestCCD";
//		return;
	}

	m_connectivityDlg.m_CCDSelectCtrl.Clear();

	std::string *devID = new std::string(W2A(ccdID));
	int index = m_connectivityDlg.m_CCDSelectCtrl.AddString(strName);
	m_connectivityDlg.m_CCDSelectCtrl.SetItemDataPtr(index, devID);


	while (true)
	{
		m_pConfigBrowser->GetNextCCD(&name, &ccdID);
		strName = W2A(name);
		strccdID = W2A(ccdID);

		if (strName.IsEmpty() && strccdID.IsEmpty())
			break;

		devID = new std::string(W2A(ccdID));
		index = m_connectivityDlg.m_CCDSelectCtrl.AddString(strName);
		m_connectivityDlg.m_CCDSelectCtrl.SetItemDataPtr(index, devID);
	}

	m_connectivityDlg.m_CCDSelectCtrl.SetCurSel(0);

	HRESULT hr;
	CLSID   clsid;

	if (m_jyCCD == NULL)
	{
		hr = CLSIDFromProgID(L"JYCCD.JYMCD", &clsid);
		std::cout << FAILED(hr) << "\n";
		hr = m_jyCCD.CoCreateInstance(clsid, NULL, CLSCTX_ALL);
		std::cout << FAILED(hr) << "\n";
		if (FAILED(hr))
			{
			TRACE("Failed to create CCD Object. Err: %ld", hr);
			return;
		}
		m_sinkPtrCCD = new CJYDeviceSink(this, m_jyCCD);
	}

}


HRESULT CiHR320Dlg::DoAcquisition(bool shutterOpen)
{
	HRESULT hr;
	m_bMeasurementStarted = true;
	m_isCCDDataReady = false;
	if (shutterOpen) hr = m_jyCCD->DoAcquisition(VARIANT_TRUE);
	else hr = m_jyCCD->DoAcquisition(VARIANT_FALSE);
	
	DWORD start = GetTickCount();
	while (m_bMeasurementStarted) {
		MSG msg;
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		Sleep(10);
	}
	return hr;
}

void CiHR320Dlg::RestartPLC()
{
	m_flowDlg.m_ExpFlowLogs.AddItem(_T("[UI-APP] PLC stopped responding. Restarting..."));
	m_connectivityDlg.m_CheckBoxPLC.SetCheck(FALSE);
	m_connectivityDlg.m_CheckBoxPLC.SetWindowText(_T("Offline"));
	std::wcout << L"PLC is not responding; restarting it...\n";
	const std::string command = "plink -batch -ssh pl-ple@" + ip_PLC +
		" -pw " + RPiPwd +
		" \"nohup python3 '/home/pl-ple/Documents/My Projects/AMTCS-System/PLC/PLC.py' > /dev/null 2>&1 &\"";
	std::system(command.c_str());
	Sleep(1000);

}

void CiHR320Dlg::ReceivedDeviceInitialised(long status, IJYEventInfo * eventInfo)
{
	// This eventInfo structure contains the "source" object that fired the initialized event
	// because you may receive this event from multiple sources.  You can extract the source ptr and 
	// use it to identify which device has been initialized, as shown below.  
	IJYDeviceReqd * sourceDevPtr;

	eventInfo->get_Source(&sourceDevPtr);

	//////// Init CCD
	if (sourceDevPtr == m_jyCCD)
	{
		USES_CONVERSION;
		int x, y;
		CComBSTR fwVer, devDesc, devName;

		CComBSTR gainStr;
		long gainToken;
		int i = 0;
		// Enumerate available Gains
		m_jyCCD->GetFirstGain(&gainStr, &gainToken);
		while (gainToken > -1)
		{
			m_gainCCD[i] = gainToken;
			m_jyCCD->GetNextGain(&gainStr, &gainToken);
			i++ ;
		}

		CComBSTR adcStr;
		long adcToken;

//		 Enumerate available Gains
		m_jyCCD->GetFirstADC(&adcStr, &adcToken);
		i = 0;
		while (adcToken > -1)
		{
			m_ADCCCD[i] = adcToken;
			m_jyCCD->GetNextADC(&adcStr, &adcToken);
			i++;
		}
		m_connectivityDlg.m_ConnectionLogs.AddItem(_T("[Synapse CCD] Initialised"));

	}

	////////// Init Mono
	if (sourceDevPtr == m_jyMono)
	{
		GetSlits();
		GetGratings();
		SetMirror();
		m_connectivityDlg.m_ConnectionLogs.AddItem(_T("[iHR320] Initialised"));

	}

	UpdateData(FALSE);

	//MessageBox(L"Initialized Received :) !");
}

void CiHR320Dlg::WaitForMono()
{
	VARIANT_BOOL bBusy = VARIANT_TRUE;

	while (SUCCEEDED(m_jyMono->IsBusy(&bBusy)) && bBusy == VARIANT_TRUE)
	{
		MSG msg;
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		Sleep(50);								// To prevent 100% CPU load
	}
}

void CiHR320Dlg::GetGratings()					// Based on SDK
{
	double currentGrating;
	CComVariant allGratings;

	m_jyMono->GetCurrentGrating(&currentGrating, &allGratings);

	SAFEARRAY* psa;
	psa = allGratings.parray;
	int numGratings = psa->rgsabound->cElements;

	CComVariant vtVal;
	double *grating;

	HRESULT hr = SafeArrayAccessData(psa, reinterpret_cast<void**> (&grating));
	_ASSERT(S_OK == hr);

	for (long i = 0; i < numGratings; i++)
	{
		m_settingsDlg.m_ListBoxDG.SetItemData(i, (DWORD)grating[i]);
	}

	int index = m_settingsDlg.m_ListBoxDG.GetCurSel();
	m_jyMono->MovetoTurret(index);

	SafeArrayUnaccessData(psa);

	double dTemp;
	m_jyMono->GetCurrentWavelength(&dTemp);
	CString text;
	text.Format(_T("%.2f"), dTemp);
	m_connectivityDlg.m_ConnectionLogs.AddItem(_T("[iHR320] Current position: ") + text + _T(" nm"));
}

void CiHR320Dlg::SetMonoDG(int grating)
{
	m_jyMono->MovetoTurret(grating);
}

void CiHR320Dlg::SetAT(double newAT)
{
	m_jyCCD->put_IntegrationTime(newAT);
}

void CiHR320Dlg::MonoMoveTo(double newPos)
{
	CString strTarget;
	strTarget.Format(_T("%.2f"), newPos);

	HRESULT hr = m_jyMono->MovetoWavelength(newPos);

	if (SUCCEEDED(hr))
	{
		PostMessageToUI(WM_USER_MONO_LOG_MESSAGE, _T("[iHR320] Moving to spectral window. Centre position: ") + strTarget + _T(" nm"));
	}
	else
	{
		PostMessageToUI(WM_USER_MONO_LOG_MESSAGE, _T("Failed to move Monochromator position..."));
	}

	VARIANT_BOOL isMonoBusy = true;
	while (isMonoBusy)
	{
		m_jyMono->IsBusy(&isMonoBusy);
		Sleep(10);
	}

	Sleep(1000);

	PostMessageToUI(WM_USER_MONO_LOG_MESSAGE, _T("[Synapse CCD] Acquiring current spectral window data..."));
}

void CiHR320Dlg::GetSlits()
{
	//get slit width from the component
	// Check to see if slit of each type is installed
	VARIANT_BOOL IsInstalled = true;
	double slitWidthFE = 0;
	CString text;
	//Front_Entrance slit
	m_jyMono->IsSubItemInstalled(Slit_Front_Entrance, &IsInstalled);
	if (IsInstalled)
	{
		m_jyMono->GetCurrentSlitWidth(Front_Entrance, &slitWidthFE);
		text.Format(_T("%.0f"), slitWidthFE*1000.0);
		m_settingsDlg.m_Slits.SetWindowTextW(text);
	}
	else
	{ 
		m_settingsDlg.m_Slits.SetWindowTextW(L"0");
		m_settingsDlg.m_Slits.EnableWindow(false);
	}
}

void CiHR320Dlg::SetSlits(double newSlits)
{
	if (m_settingsDlg.m_Slits.IsWindowEnabled())
	{
		m_jyMono->MovetoSlitWidth(Front_Entrance, newSlits);
	}

}

void CiHR320Dlg::SetMirror()
{
	MirrorLocation locationEX;
	VARIANT_BOOL bIsInstalled = true;

	//Exit mirror
	m_jyMono->IsSubItemInstalled(Mirror_Exit, &bIsInstalled);
	if (bIsInstalled)
	{
		m_jyMono->GetCurrentMirrorPosition(ExitMirror, &locationEX);
		switch (locationEX)
		{
		case Front:
			break;
		case Side:
			m_jyMono->MovetoMirrorPosition(ExitMirror, Front);
			break;
		default:
			break;
		}
	}
	else // if its not installed, let user know (expected installed)
	{
		MessageBox(L"Equipment configuration problem:", L"Exit mirror expected, but not detected...");
	}

}

void CiHR320Dlg::ReceivedDeviceStatus(long status, IJYEventInfo * eventInfo)
{
	CComBSTR desc;
	eventInfo->get_Description(&desc);
	CString strValue(desc);
	m_flowDlg.m_ExpFlowLogs.AddItem(strValue);
	UpdateData(FALSE);
}

void CiHR320Dlg::ReceivedDeviceUpdate(long updateType, IJYEventInfo * eventInfo)
{
	switch (updateType)
	{
		case 100: // 100 - updateType corresponding to CCD (after receiving new data) 
		{
			CComVariant data, xData;
			CComPtr<IJYResultsObject> resultObject = NULL;
			eventInfo->GetResult(&resultObject);
			if (resultObject == NULL) {
//				m_flowDlg.m_ExpFLowLogs.AddItem(CString(_T("[CCD] Warning: no data received.")));
				m_bMeasurementStarted = FALSE;
				m_isCCDDataReady = true;
				return; 
			}
			m_AcqDataObj = NULL;
			resultObject->GetFirstDataObject(&m_AcqDataObj);
			if (m_AcqDataObj == NULL) {
//				m_flowDlg.m_ExpFLowLogs.AddItem(CString(_T("[CCD] Warning: no data received.")));
				m_bMeasurementStarted = FALSE;
				m_isCCDDataReady = true;
				return;
			}
			CComPtr<IJYDeviceReqd> pSourceDev;
			eventInfo->get_Source(&pSourceDev);

			if (pSourceDev != NULL)
			{
			
				CComPtr<IJYCCDReqd> pCCD;			// To cast the source as a Multi-Channel Detector
				pSourceDev->QueryInterface(__uuidof(IJYCCDReqd), (void**)&pCCD);

				if (pCCD != NULL)
				{
					// Now you can call GetAxisAs directly on the MCD interface
					CComPtr<IJYAxis> pXAxis;
					pCCD->GetAxisAs(m_AcqDataObj, 1, jyDATWavelength, &pXAxis);
					if (pXAxis != NULL)
					{
			
						pXAxis->GetValuesByArray(&xData);			// Retrieving X-values into CComVariant

						if ((xData.vt & VT_ARRAY) && xData.parray != NULL)
						{
							double* pRawXData;
							SafeArrayAccessData(xData.parray, (void**)&pRawXData);

							long lBoundX, uBoundX;
							SafeArrayGetLBound(xData.parray, 1, &lBoundX);
							SafeArrayGetUBound(xData.parray, 1, &uBoundX);

							currentData.wavelengths.assign(pRawXData, pRawXData + (uBoundX - lBoundX + 1));

							SafeArrayUnaccessData(xData.parray);
						}
					}
				}
			}
			IJYAxis *pYAxis = NULL;

			CComBSTR dataDesc;

			m_AcqDataObj->get_Description(&dataDesc);
			m_AcqDataObj->GetDataAsArray(&data);
			if ((data.vt & VT_ARRAY) && data.parray != NULL)
			{
				long* pRawData;
				SafeArrayAccessData(data.parray, (void**)&pRawData);

				long lBound, uBound;					// bounds of data array
				SafeArrayGetLBound(data.parray, 1, &lBound);
				SafeArrayGetUBound(data.parray, 1, &uBound);
				currentData.pixelCount = uBound - lBound + 1;
			
				currentData.intensities.assign(pRawData, pRawData + currentData.pixelCount);

				SafeArrayUnaccessData(data.parray);
			}

			Sleep(500);
			m_bMeasurementStarted = FALSE;
			m_isCCDDataReady = true;

		}
	break;
	default:
		break;
	}
	//MessageBox(L"Update Received :) !");

}

void CiHR320Dlg::ReceivedDeviceCriticalError(long status, IJYEventInfo * eventInfo)
{
}

ExperimentParameters CiHR320Dlg::GetExperimentParameters()
{
	return m_settingsDlg.GetExperimentParameters();
}

int CiHR320Dlg::GetExperimentProgressIndex()
{
	return m_settingsDlg.experimentState.experimentProgressIndex;
}

void CiHR320Dlg::AddNewT(int T)								// Adds extra data point to running experiment				
{
	int previousT;
	if (m_settingsDlg.experimentState.experimentProgressIndex < 0) {
		previousT = m_settingsDlg.m_VSListBox_T.GetItemData(0);
	}
	else previousT = m_settingsDlg.m_VSListBox_T.GetItemData(m_settingsDlg.experimentState.experimentProgressIndex);

	
	int currentT = m_settingsDlg.m_VSListBox_T.GetItemData(m_settingsDlg.experimentState.experimentProgressIndex + 1);
	int lastT = m_settingsDlg.m_VSListBox_T.GetItemData(m_settingsDlg.experimentState.experimentLength - 1);
	CString strT;
	strT.Format(_T("%d"), T);

	ExperimentParameters newParams = GetExperimentParameters();

	if (m_settingsDlg.experimentState.experimentLength == 1) {
		m_settingsDlg.m_VSListBox_T.AddItem(strT, T);
	}
	else if (currentT > lastT && previousT > T) {			// Temperatures in descending order
		m_settingsDlg.m_VSListBox_T.AddItem(strT, T);		
		m_settingsDlg.m_VSListBox_T.SortT(FALSE);			// Sort list with new T in descending order
	}
	else if (currentT < lastT && previousT < T) {			// Temperatures in ascending order
		m_settingsDlg.m_VSListBox_T.AddItem(strT, T);
		m_settingsDlg.m_VSListBox_T.SortT(TRUE);			// Sort list with new T in ascending order
	}
	else {
		m_flowDlg.m_ExpFlowLogs.AddItem(_T("New temperature (") + strT + _T(" K) is out of allowed range"));
		return;
	}
	m_askUser.s_question.Format(_T("Are you sure you want to add an over %d K temperature?"), HT);
	if (T <= HT || m_askUser.DoModal() == IDOK) {
		newParams.Ts = m_settingsDlg.m_VSListBox_T.GetAllItemTs();
		m_settingsDlg.experimentState.setExpParams(newParams);
		m_settingsDlg.experimentState.experimentLength++;
		m_settingsDlg.OnBnClickedStart();
		m_flowDlg.m_ExpFlowLogs.AddItem(_T("Adding new temperature (") + strT + _T(" K) requested."));
	}
	else {
		m_flowDlg.m_ExpFlowLogs.AddItem(_T("Adding new temperature: (") + strT + _T(" K) cancelled."));
	}
}

void CiHR320Dlg::PostMessageToUI(UINT message, CString logMessage) {
	// Basic guard: ensure we are only posting user-defined messages
	if (message < WM_USER) return;

	CString* pMsg = new CString(logMessage);
	if (!::PostMessage(this->m_hWnd, message, 0, (LPARAM)pMsg))
		delete pMsg;			// Delete pointer here if no one picks up the message (to avoid memory leaking)
}

void CiHR320Dlg::EnableExpSettDlg()
{
	EnableDlg(&m_settingsDlg, TRUE);
	EnableDlg(&m_connectivityDlg, FALSE);
	EnableDlg(&m_flowDlg, FALSE);
}

void CiHR320Dlg::EnableExpFlowDlg()
{
	EnableDlg(&m_flowDlg, TRUE);
	EnableDlg(&m_connectivityDlg, FALSE);
	EnableDlg(&m_settingsDlg, FALSE);
}

void CiHR320Dlg::DisableConnDlg()
{
	EnableDlg(&m_connectivityDlg, FALSE);
	EnableDlg(&m_flowDlg, FALSE);

	m_jyCCD->SetMono(m_jyMono, VARIANT_TRUE);
	m_jyCCD->SetDefaultUnits(jyutWavelength, jyuNanometers);
	m_jyCCD->SetDefaultUnits(jyutDataUnits, jyuCounts);
	m_jyCCD->GetDefaultUnits(jyutWavelength, &eUnits, &vUnits);
}

void CiHR320Dlg::DisableExpSettDlg()
{
	EnableDlg(&m_settingsDlg, FALSE);

}

void CiHR320Dlg::SetExpProgress()
{
	int done = m_settingsDlg.experimentState.experimentProgressIndex;
	int total = m_settingsDlg.experimentState.experimentLength;

	if (done > -1) m_flowDlg.m_repeatPreviousTBtn.EnableWindow(TRUE);
	else m_flowDlg.m_repeatPreviousTBtn.EnableWindow(FALSE);

	if (total <= 0) total = 1;					// To avoid invalid range
	m_flowDlg.m_expProgressBar.SetRange(0, total);
	m_flowDlg.m_expProgressBar.SetPos(done + 1);
}

void CiHR320Dlg::RepeatPreviousT()
{
	if (m_settingsDlg.experimentState.experimentProgressIndex > -1) {
		m_settingsDlg.experimentState.experimentProgressIndex--;
		EnableDlg(&m_flowDlg, FALSE);
		m_settingsDlg.OnBnClickedStart();
	}
	else {
		AfxMessageBox(_T("There is no 'previous setpoint' yet!"));
	}
}

LRESULT CiHR320Dlg::OnMonoLogMessage(WPARAM wParam, LPARAM lParam)
{
	CString* pStr = reinterpret_cast<CString*>(lParam);
	if (pStr)
	{
		m_flowDlg.m_ExpFlowLogs.AddItem(*pStr);
		delete pStr;
	}
	return 0;
}

void CiHR320Dlg::EnableDlg(CWnd* pTargetDlg, BOOL bEnable)
{
	
	pTargetDlg->EnableWindow(bEnable);					// En/disable the window itself

	CWnd* pChild = pTargetDlg->GetWindow(GW_CHILD);		// Get first child
	while (pChild)										// Iterate though all children
	{
		pChild->EnableWindow(bEnable);
		pChild->RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
		pChild = pChild->GetWindow(GW_HWNDNEXT);
	}
}

void CiHR320Dlg::StartTimer(UINT_PTR nIDEvent, int _sec) {
	SetTimer(nIDEvent, 100 + 1000 * _sec, NULL);
}

void CiHR320Dlg::StopTimer(UINT_PTR nIDEvent) {
	if (nIDEvent == TIMER_EXP_PAUSE_CONTINUE) {
		m_flowDlg.m_pauseResumeBtn.SetWindowTextW(m_flowDlg.m_nextUserAction);
		if (m_flowDlg.m_nextUserAction == _T("Continue")) {
			m_flowDlg.m_cancelExp.EnableWindow(FALSE);
			m_flowDlg.m_repeatPreviousTBtn.EnableWindow(FALSE);
			m_flowDlg.m_addNewTBtn.EnableWindow(FALSE);
		}
		else {
			m_flowDlg.m_cancelExp.EnableWindow(TRUE);
			m_flowDlg.m_repeatPreviousTBtn.EnableWindow(TRUE);
			m_flowDlg.m_addNewTBtn.EnableWindow(TRUE);
		}
	}
	KillTimer(nIDEvent);
}

void CiHR320Dlg::OnTimer(UINT_PTR nIDEvent) {

	if (nIDEvent == TIMER_PLC_CHECK) {
		m_connectivityDlg.m_CheckBoxPLC.SetCheck(FALSE);
		m_connectivityDlg.m_CheckBoxPLC.SetWindowText(_T("Not Responsive"));
		m_connectivityDlg.m_ConnectionLogs.AddItem(_T("PLC Timeout Error"));
		KillTimer(nIDEvent);
	}
	else if (nIDEvent == TIMER_ALL_CHECK) {
		if (m_availableDeviceCount >= 4) {
			if (GetExperimentProgressIndex() == -1) {
				EnableExpSettDlg();						// enable Experimental Setting Dialog if everything is connected
				DisableConnDlg();						// ... and disable Connectivity Dialog (not necessary anymore)
				m_connectivityDlg.m_ConnectionLogs.AddItem(_T("-------------------------------------------------"));
				if (g_isPLCOnAtStart) {
					m_connectivityDlg.m_ConnectionLogs.AddItem(_T("Hardware is ready. Please wait for experiment status on PLC..."));
				}
				else {
					m_connectivityDlg.m_ConnectionLogs.AddItem(_T("Hardware is ready. Please switch to Experiment Settings Tab."));
				}
				m_connectivityDlg.m_ConnectionLogs.Invalidate();
				m_connectivityDlg.m_ConnectionLogs.UpdateWindow();
			}
			else {
				CString msg = _T("CONTINUE_EXP");		// Proceed to experiment when recieved from PLC
				PostMessageToUI(WM_UPDATE_SYSTEM_EVENT, msg);
			}
			if (g_isPLCOnAtStart) {
				if (!(SendTCPMessage(this, ip_PLC, port_PLC, "EXPERIMENT?"))) {
					AfxMessageBox(_T("TCP connection failed: experiment status could not be requested."));
					StopTimer(TIMER_EXP_REQUESTED_BY_UI);
					Sleep(2000);
					SelectTab(1);
				}
				else {
					m_connectivityDlg.m_ConnectionLogs.AddItem(_T("Experiment state requested. Waiting for PLC response..."));
				}
			}
			else {
				Sleep(2000);
				SelectTab(1);
			}
		}
		else {
			m_connectivityDlg.m_connectBtn.EnableWindow(TRUE);
			m_connectivityDlg.m_connectBtn.SetWindowTextW(_T("Connect to equipment"));
			m_connectivityDlg.m_ConnectionLogs.AddItem(_T("-------------------------------------------------"));
			m_connectivityDlg.m_ConnectionLogs.AddItem(_T("Not all hardware is ready! Please, check your equipment."));
			m_connectivityDlg.m_ConnectionLogs.Invalidate();
			m_connectivityDlg.m_ConnectionLogs.UpdateWindow();
		}
		KillTimer(nIDEvent);
	}
	else if (nIDEvent == TIMER_EXP_SENT) {
		AfxMessageBox(_T("PLC did not respond. Please check RPi\n"));
		KillTimer(nIDEvent);
	}
	else if (nIDEvent == TIMER_PLC_STOP) {
		m_isPLCConfirmedOff = true;
		std::cout << "PLC is not responsive. Exiting anyway...\n";
		KillTimer(nIDEvent);
		Sleep(2000);

	}
	else if (nIDEvent == TIMER_EXP_PAUSE_CONTINUE) {
		AfxMessageBox(_T("PLC is not responsive. Please check RPi\n"));
		KillTimer(nIDEvent);
	}
	else if (nIDEvent == TIMER_EXP_CANCEL) {
		AfxMessageBox(_T("PLC is not responsive. Please check RPi\n"));
		KillTimer(nIDEvent);
	}
}
