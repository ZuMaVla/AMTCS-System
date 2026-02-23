
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
	m_bMonoInitialized = FALSE;

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

//CComPtr<IJYMonoReqd> CiHR320Dlg::GetMonoPtr()
//{
//	return m_jyMono;
//}


LRESULT CiHR320Dlg::OnUpdateSystemStatus(WPARAM wParam, LPARAM lParam)
{
	std::string* device = reinterpret_cast<std::string*>(lParam);

	m_connectivityDlg.UpdateSystemStatusUI(*device);

	delete device;   // free the memory after using
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
	m_connectivityDlg.m_ConnectionLogs.AddItem(L"Initialising CCD...Please wait");
	hr = m_jyCCD->Initialize((CComVariant)false, (CComVariant)false);
	if (FAILED(hr))
	{
		m_connectivityDlg.m_ConnectionLogs.AddItem(L"Check Hardware and Try Again...");
		return FALSE;
	}
	m_connectivityDlg.m_ConnectionLogs.AddItem(L"CCD Initialised :)");
	return TRUE;
}

//=_=_=_=_=_=_=_=_=_=_=_=_=_=_=_=_=_=_=_=_=_=_=_=_=_=_=_=_=_=_=_=_=_=_=_=_=_=_=_=_=_=_=_=_=
//	ORIG AUTHOR:	J. Martin
//
//	PARAMETERS:
//
//	DESCRIPTION:	Attempt to connect to the selected CCD Device [and Mono - MZ]
//
//	RETURNS:
//
//	NOTES:
//_______________
std::array<BOOL, 2> CiHR320Dlg::ConnectMonoAndCCD()
{
	std::array<BOOL, 2> result = { FALSE, FALSE };
	HRESULT hr = S_OK;
	int nSel;

//*******************************---CCD---****************************************************************************

	//if (m_connectivityDlg.m_CCDSelectCtrl.GetCount()) 
	//{
	//	nSel = m_connectivityDlg.m_CCDSelectCtrl.GetCurSel();
	//	auto* selectedCCD = static_cast<std::string*>(m_connectivityDlg.m_CCDSelectCtrl.GetItemDataPtr(nSel));
	//	std::cout << "Selected CCD: " << selectedCCD << "\n";
	//	m_jyCCD->put_Uniqueid((CComBSTR)selectedCCD->c_str());
	//	m_jyCCD->Load();
	//	hr = m_jyCCD->OpenCommunications();

	//	if (FAILED(hr))
	//	{
	//		MessageBox(L"Check Hardware and Try Again...");
	//	}
	//	else
	//	{
	//		if (!FAILED(m_jyCCD->Initialize((CComVariant)false, (CComVariant)false))) {
	//			result[0] = TRUE;
	//		}
	//	}
	//}
	result[0] = ConnectAndInitCCD();

					// Real inintialisation (no emulation)

//*******************************---Mono---****************************************************************************

	// Get the user selected id...
	if (m_connectivityDlg.m_CCDSelectCtrl.GetCount())
	{

		CString selectedMono;
		nSel = m_connectivityDlg.m_MonoSelectCtrl.GetCurSel();
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
			if (!FAILED(m_jyMono->Initialize((CComVariant)m_bMonoInitialized, (CComVariant)false)))
			{
				result[1] = TRUE;
				m_bMonoInitialized = true;
			}
		}
	}

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
		strName = L"testMono";
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


HRESULT CiHR320Dlg::DoAcquisition()
{
	UpdateData(FALSE);
	//// 
	//// Non Threaded...
	////

	IJYResultsObject *resultObj;
	//IJYDataObject *dataObj;
	//VARIANT_BOOL isBusy = true;
	//CComVariant data;
	//m_jyCCD->StartAcquisition(VARIANT_TRUE);
	//while (isBusy)
	//{
	//	m_jyCCD->AcquisitionBusy(&isBusy);
	//	// PUMP MESSAGES: This allows the COM component to update its state
	//	MSG msg;
	//	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	//	{
	//		TranslateMessage(&msg);
	//		DispatchMessage(&msg);
	//	}

	//	Sleep(10); // Don't peg the CPU at 100%
	//}
	//m_jyCCD->GetResult(&resultObj);

	//HRESULT hr = resultObj->GetFirstDataObject(&dataObj);
	//dataObj->GetDataAsArray(&data);

	HRESULT hr = m_jyCCD->DoAcquisition(VARIANT_TRUE);
	DWORD start = GetTickCount();
	while (GetTickCount() - start < 3000) {
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

void CiHR320Dlg::ReceivedDeviceInitialized(long status, IJYEventInfo * eventInfo)
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
		jyUnits eUnits;
		VARIANT vUnits;
		CString sUnits;

//		m_jyCCD->GetDefaultUnits(jyutTime, &eUnits, &vUnits);		// Acquisition time
//		sUnits = vUnits.bstrVal;

//		m_jyCCD->get_Name(&devName);
//		m_name = devName;

		CComBSTR gainStr;
		long gainToken;
		int addedIndex;
		// Enumerate available Gains
		//m_jyCCD->GetFirstGain(&gainStr, &gainToken);
		//while (gainToken > -1)
		//{
		//	addedIndex = m_gainsCtrl.AddString(W2A(gainStr));
		//	m_gainsCtrl.SetItemData(addedIndex, gainToken);
		//	m_jyCCD->GetNextGain(&gainStr, &gainToken);
		//}
//		m_gainsCtrl.SetCurSel(0);

		CComBSTR adcStr;
		long adcToken;

		// Enumerate available Gains
		//m_jyCCD->GetFirstADC(&adcStr, &adcToken);
		//while (adcToken > -1)
		//{
		//	addedIndex = m_adcCtrl.AddString(W2A(adcStr));
		//	m_adcCtrl.SetItemData(addedIndex, adcToken);
		//	m_jyCCD->GetNextADC(&adcStr, &adcToken);
		//}
		//m_adcCtrl.SetCurSel(0);
		m_flowDlg.m_ExpFLowLogs.AddItem(L"Initialized Received from CCD!");

	}

	////////// Init Mono
	if (sourceDevPtr == m_jyMono)
	{
//		GetSlits();
//		GetGratings();
//		GetMirrors();
		//		GetMonoCommSettings();
		m_flowDlg.m_ExpFLowLogs.AddItem(L"Initialized Received from Mono!");
	}

	UpdateData(FALSE);

	//MessageBox(L"Initialized Received :) !");
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
		CComVariant data;
		IJYResultsObject *resultObject = NULL;
		CComBSTR dataDesc;
		eventInfo->GetResult(&resultObject);
		resultObject->GetFirstDataObject(&m_AcqDataObj);
		m_AcqDataObj->get_Description(&dataDesc);
		m_AcqDataObj->GetDataAsArray(&data);
		m_flowDlg.m_ExpFLowLogs.AddItem(L"Data Update received...");
		m_flowDlg.m_ExpFLowLogs.AddItem(L"Acquisition Completed.");
		UpdateData(FALSE);
	}
	break;
	default:
		break;
	}
	MessageBox(L"Update Received :) !");

}

void CiHR320Dlg::ReceivedDeviceCriticalError(long status, IJYEventInfo * eventInfo)
{
}

