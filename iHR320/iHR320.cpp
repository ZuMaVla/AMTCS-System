
#pragma comment(lib, "ws2_32.lib")

#include "stdafx.h"
#include "iHR320.h"
#include "iHR320Dlg.h"
#include "TCPtoRPi.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <cstdlib>
#include "Security.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

bool g_isPLCOnAtStart = false;


// CiHR320App

BEGIN_MESSAGE_MAP(CiHR320App, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CiHR320App construction

CiHR320App::CiHR320App()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CiHR320App object

CiHR320App theApp;

const GUID CDECL BASED_CODE _tlid =
		{ 0xAD7C3530, 0x5259, 0x4592, { 0xA3, 0xC2, 0xA3, 0x9C, 0x43, 0x4A, 0x12, 0xFA } };
const WORD _wVerMajor = 1;
const WORD _wVerMinor = 0;
CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
END_OBJECT_MAP()


bool isPLCRunning(const std::string& scriptName) {
	// -f checks the full command line (needed for Python scripts)
	// > nul (Windows) or > /dev/null (Linux) hides the output
	std::string checkCmd = "plink -batch -ssh pl-ple@" + ip_PLC + " -pw " + RPiPwd + " \"pgrep -f " + scriptName + "\" > nul";
	bool result = std::system(checkCmd.c_str()) == 0;		// std::system returns 0 if pgrep finds the process
	if (result) std::wcout << L"PLC already running...\n";

	return (result);		
}

// CiHR320App initialization

BOOL CiHR320App::InitInstance()
{
	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();

	AllocConsole();
	FILE* fp;
	freopen_s(&fp, "CONOUT$", "w", stdout);
	freopen_s(&fp, "CONOUT$", "w", stderr);
	freopen_s(&fp, "CONIN$", "r", stdin);

	std::string pingCmd = "ping -n 1 -w 1000 " + ip_PLC + " > nul";
	if (std::system(pingCmd.c_str()) != 0) {
		std::wcout << L"Error: RPi is offline. Check cables/power." << std::endl;
	}
	else {
		if (!isPLCRunning("PLC.py")) {
			std::wcout << L"PLC is not running; starting it...\n";
			const std::string command = "plink -batch -ssh pl-ple@" + ip_PLC + 
				" -pw " + RPiPwd +
				" \"nohup python3 '/home/pl-ple/Documents/My Projects/AMTCS-System/PLC/PLC.py' > /dev/null 2>&1 &\"";
			std::system(command.c_str());
			Sleep(1000);
		}
		else g_isPLCOnAtStart = true;
	}

	// Initialize OLE libraries
	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}
	if (!InitATL())
		return FALSE;

	AfxEnableControlContainer();

	// Create the shell manager, in case the dialog contains
	// any shell tree view or shell list view controls.
	CShellManager *pShellManager = new CShellManager;

	// Activate "Windows Native" visual manager for enabling themes in MFC controls
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));
	// Parse command line for automation or reg/unreg switches.
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	// App was launched with /Embedding or /Automation switch.
	// Run app as automation server.
	if (cmdInfo.m_bRunEmbedded || cmdInfo.m_bRunAutomated)
	{
		// Register class factories via CoRegisterClassObject().
		COleTemplateServer::RegisterAll();
		_Module.RegisterClassObjects(CLSCTX_LOCAL_SERVER, REGCLS_MULTIPLEUSE);
	}
	// App was launched with /Unregserver or /Unregister switch.  Remove
	// entries from the registry.
	else if (cmdInfo.m_nShellCommand == CCommandLineInfo::AppUnregister)
	{
		COleObjectFactory::UpdateRegistryAll(FALSE);
		AfxOleUnregisterTypeLib(_tlid, _wVerMajor, _wVerMinor);
		return FALSE;
	}
	// App was launched standalone or with other switches (e.g. /Register
	// or /Regserver).  Update registry entries, including typelibrary.
	else
	{
		COleObjectFactory::UpdateRegistryAll();
		AfxOleRegisterTypeLib(AfxGetInstanceHandle(), _tlid);
		if (cmdInfo.m_nShellCommand == CCommandLineInfo::AppRegister)
			return FALSE;
	}


	// Winsock init

	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		std::cerr << "WSAStartup failed\n";
	}

	CiHR320Dlg dlg;
	m_pMainWnd = &dlg;

	dlg.DoModal();

	StopMainLogicThread();					// Stopping communication with PLC 


	

	WSACleanup();							// WSA cleanup

	
	
	if (pShellManager != NULL)
	{
		delete pShellManager;				// Delete the shell manager created above.
	}

#ifndef _AFXDLL
	ControlBarCleanUp();
#endif

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}

int CiHR320App::ExitInstance()
{
	AfxOleTerm(FALSE);

	if (m_bATLInited)
	{
		CoUninitialize();
		m_bATLInited = FALSE;
	}

	return CWinApp::ExitInstance();
}

BOOL CiHR320App::InitATL()
{
	// If you call InitATL multiple times, keep it idempotent.
	if (m_bATLInited)
		return TRUE;

	// Match the vendor example first:
	HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);

	if (hr == RPC_E_CHANGED_MODE)
	{
		// COM is already initialized on this thread with a different apartment model (likely STA).
		// You cannot "switch" the apartment for this thread.
		//
		// If you are an MFC GUI app, this often happens because MFC initialized COM as STA.
		// In that case, do NOT call CoUninitialize for this path (because you didn't init it).
		std::wcout << L"COM already initialized with different mode (RPC_E_CHANGED_MODE). "
			L"Continuing without reinitializing.\n";
		return TRUE;
	}

	if (FAILED(hr))
	{
		std::wcout << L"CoInitializeEx failed: 0x" << std::hex << hr << L"\n";
		return FALSE;
	}

	m_bATLInited = TRUE;
	return TRUE;
}