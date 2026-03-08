
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
	ON_MESSAGE(WM_UPDATE_SYSTEM_STATUS, &CiHR320Dlg::OnUpdateSystemStatus)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_MAIN, &CiHR320Dlg::OnTabSelChange)
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

LRESULT CiHR320Dlg::OnPutLog(WPARAM wParam, LPARAM lParam)
{
	CString* pStr = reinterpret_cast<CString*>(lParam);
	if (pStr)
	{
		CString msg = *pStr;
		if (msg.Left(2) == _T("T=")) {
			msg = _T("[TC] Current temperature received: ") + msg.Mid(2) + _T(" K");

			m_connectivityDlg.m_ConnectionLogs.AddItem(msg);
		}
		else if (msg.Left(3) == _T("CT=")) {
			msg = _T("[TC] Target temperature reached. Current T: ") + msg.Mid(4) + _T(" K");

			m_flowDlg.m_ExpFLowLogs.AddItem(msg);
		}
		else if (msg.Left(3) == _T("SAd")) {
			msg = _T("[TC] Current temperature received: ") + msg.Mid(2) + _T(" K");

			m_flowDlg.m_ExpFLowLogs.AddItem(L"[iHR320] Spectrum acquisition completed; data saved.");
		}

		delete pStr;
	}
	return 0;
}

BOOL CiHR320Dlg::ConnectAndInitCCD()
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

BOOL CiHR320Dlg::ConnectAndInitMono()
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

void CiHR320Dlg::SetCCDParams()
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

void CiHR320Dlg::LoadMonos()
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

void CiHR320Dlg::LoadCCDs()
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
	//while (GetTickCount() - start < 3000) {
	//	MSG msg;
	//	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	//	{
	//		TranslateMessage(&msg);
	//		DispatchMessage(&msg);
	//	}
	//	Sleep(10);
	//}

	return hr;
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

void CiHR320Dlg::GetGratings()
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
		PostMessageToUI(WM_USER_MONO_LOG_MESSAGE, L"Mono moving to the new position..." + strTarget);
	}
	else
	{
		PostMessageToUI(WM_USER_MONO_LOG_MESSAGE, L"Failed to move Monochromator position...");
	}

	VARIANT_BOOL isMonoBusy = true;
	while (isMonoBusy)
	{
		m_jyMono->IsBusy(&isMonoBusy);
		Sleep(10);
	}

	PostMessageToUI(WM_USER_MONO_LOG_MESSAGE, L"Mono is at the new position: " + strTarget + L" nm");
	
	Sleep(1000);

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
	m_flowDlg.m_ExpFLowLogs.AddItem(strValue);
	UpdateData(FALSE);
}

void CiHR320Dlg::ReceivedDeviceUpdate(long updateType, IJYEventInfo * eventInfo)
{
	switch (updateType)
	{
		case 100: // data
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

void CiHR320Dlg::PostMessageToUI(UINT message, CString logMessage) {
	// Basic guard: ensure we are only posting user-defined messages
	if (message < WM_USER) return;

	CString* pMsg = new CString(logMessage);
	if (!::PostMessage(this->m_hWnd, message, 0, (LPARAM)pMsg))
		delete pMsg;
}

void CiHR320Dlg::EnableExpSettDlg()
{
	EnableDlg(&m_settingsDlg, TRUE);
}

void CiHR320Dlg::DisableConnDlg()
{
	EnableDlg(&m_connectivityDlg, FALSE);
	m_jyCCD->SetMono(m_jyMono, VARIANT_TRUE);
	m_jyCCD->SetDefaultUnits(jyutWavelength, jyuNanometers);
	m_jyCCD->SetDefaultUnits(jyutDataUnits, jyuCounts);
	m_jyCCD->GetDefaultUnits(jyutWavelength, &eUnits, &vUnits);

}

LRESULT CiHR320Dlg::OnMonoLogMessage(WPARAM wParam, LPARAM lParam)
{
	CString* pStr = reinterpret_cast<CString*>(lParam);
	if (pStr)
	{
		m_connectivityDlg.m_ConnectionLogs.AddItem(*pStr);
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
