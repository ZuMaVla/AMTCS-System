// iHR320Dlg.h : header file
// C:\Users\PL&PLE\Documents\Lab software\AMTCS-System

#pragma once

#include <afxcmn.h>
#include "CiHR320ConnectivityDlg.h"
#include "CiHR320SettingsDlg.h"
#include "CiHR320FlowDlg.h"
#include "AskUser.h"
#include "Resource.h"
#include "DataAcquisition.h"

// messages
#define WM_UPDATE_SYSTEM_STATUS (WM_APP + 1)
#define WM_USER_MONO_LOG_MESSAGE (WM_USER + 110)
#define WM_USER_LOG_MESSAGE (WM_USER + 111)
#define WM_UPDATE_SYSTEM_EVENT (WM_USER + 112)


// timers
#define TIMER_PLC_CHECK 101
#define TIMER_ALL_CHECK 102
#define TIMER_EXP_SENT 103
#define TIMER_PLC_STOP 104
#define TIMER_EXP_PAUSE_CONTINUE 105
#define TIMER_EXP_CANCEL 106
#define TIMER_EXP_REQUESTED_BY_UI 107


// forward declarations
class CiHR320DlgAutoProxy;						// Standard MFC template
class CJYDeviceSink;							// From SDK example

struct CCDThreadData {							// CCD data container (last measurement for export)
	std::vector<long> intensities;
	std::vector<double> wavelengths;
	long pixelCount;
};

// CiHR320Dlg dialog
class CiHR320Dlg : public CDialogEx
{
	DECLARE_DYNAMIC(CiHR320Dlg);
	friend class CiHR320DlgAutoProxy;

// Dialog Data
	enum { IDD = IDD_IHR320_MAIN_DLG };

public:
// Construction
	CiHR320Dlg(CWnd* pParent = NULL);	// standard constructor
	virtual ~CiHR320Dlg();
// 
	CAskUser m_askUser;					// customised "yes/no" dialog
	CCDThreadData currentData;
	std::string jsonState = "";
	std::string GetLocalIP();
	CString GetCurrentDir();
	void SelectTab(int index);			// Programmatic switching between UI App subdialogs

	std::array<double, 5> GetCentresWL(int startWL, int DGRangeNo);		// Calculates Mono positions for desired spectral range
	int m_availableDeviceCount;
	int m_missedPLCHeartbeatCount = 0;
	void RestartPLC();
	void ReceivedDeviceInitialised(long status, IJYEventInfo *eventInfo);		// From SDK example
	void ReceivedDeviceStatus(long status, IJYEventInfo *eventInfo);			// From SDK example
	void ReceivedDeviceUpdate(long status, IJYEventInfo *eventInfo);			// From SDK example
	void ReceivedDeviceCriticalError(long status, IJYEventInfo *eventInfo);		// From SDK example
	void WaitForMono();
	ExperimentParameters GetExperimentParameters();								// Getter for exp params
	int GetExperimentProgressIndex(); 
	AcquisitionParameters m_acqParams;											// Current CCD params
	void AddNewT(int T);														// Adds extra data point to running experiment	
	void PostMessageToUI(UINT message, CString logMessage);
	void EnableExpSettDlg();
	void EnableExpFlowDlg();
	void DisableConnDlg();
	void DisableExpSettDlg();
	bool m_bMeasurementStarted = false;											// Flag of started acquisition
	bool m_isCCDDataReady = false;												// Flag of data ready (after an acquisition)						
	bool m_isPLCConfirmedOff = false;
	bool m_isMonoInitialised;
	double m_currT = 294.0;

	void StartTimer(UINT_PTR nIDEvent, int _sec);
	void StopTimer(UINT_PTR nIDEvent);
	afx_msg void OnTimer(UINT_PTR nIDEvent);									// Ensure this is in the Message Map

	void SetExpProgress();														// Exp progress bar update		
	void RepeatPreviousT();
	BOOL
		isExitEnabled = FALSE,
		m_isNextExpRefused = FALSE,
		m_isExperimentStarted = FALSE,											// Flag of experiment running	
		CanExit();

	CTabCtrl m_tab;
	CiHR320ConnectivityDlg m_connectivityDlg;
	CiHR320SettingsDlg m_settingsDlg;
	CiHR320FlowDlg m_flowDlg;

protected:

	HICON m_hIcon;
	CiHR320DlgAutoProxy* m_pAutoProxy;

	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnClose();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnTabSelChange(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg LRESULT OnUpdateSystemEvent(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnUpdateSystemStatus(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnPutLog(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnMonoLogMessage(WPARAM wParam, LPARAM lParam);
	void EnableDlg(CWnd * pTargetDlg, BOOL bEnable);
	DECLARE_MESSAGE_MAP()
//---------------------------------------------SDK--------------------------------------------------
	CString m_monoArray[10][2];													// From SDK
	long m_gainCCD[3], m_ADCCCD[3];												// from SDK					
	bool m_bDetectorInitialized;												// from SDK		
	CComPtr<IJYMonoReqd> m_jyMono;												// from SDK		
	CComPtr<CJYDeviceSink> m_sinkPtrMono;										// from SDK		
	CComPtr<IJYCCDReqd> m_jyCCD;												// from SDK		
	CComPtr<CJYDeviceSink> m_sinkPtrCCD;										// from SDK		
	CComPtr<IJYConfigBrowerInterface> m_pConfigBrowser;							// from SDK		
	CComPtr<IJYDataObject> m_AcqDataObj;										// Holds data gathered by acquisition
	void LoadMonos();
	void LoadCCDs();
	BOOL ConnectAndInitCCD();
	BOOL ConnectAndInitMono();

public:
	jyUnits eUnits;
	VARIANT vUnits;
	CString sUnits;
	void GetGratings();
	void GetSlits();
	void SetMonoDG(int grating);
	void SetSlits(double newSlits);
	void SetMirror();
	void SetAT(double newAT);
	void MonoMoveTo(double newPos);
	std::array<BOOL, 2> ConnectMonoAndCCD();
	HRESULT DoAcquisition(bool shutterOpen = true);
	void SetCCDParams();
	bool m_simulationMode = false;

};
