
// iHR320Dlg.h : header file
// C:\Users\PL&PLE\Documents\Lab software\AMTCS-System

#pragma once

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

class CiHR320DlgAutoProxy;
#include <afxcmn.h>
#include "CiHR320ConnectivityDlg.h"
#include "CiHR320SettingsDlg.h"
#include "CiHR320FlowDlg.h"
#include "AskUser.h"
#include "Resource.h"

// forward declarations
class CJYDeviceSink; 



struct CCDThreadData {							// CCD data container for export
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
	CAskUser m_askUser;
	CCDThreadData currentData;
	std::string GetLocalIP();
	CString GetCurrentDir();

	std::array<double, 5> GetCentresWL(int startWL, int DGRangeNo);
	int m_availableDeviceCount;
	void ReceivedDeviceInitialised(long status, IJYEventInfo *eventInfo);
	void ReceivedDeviceStatus(long status, IJYEventInfo *eventInfo);
	void ReceivedDeviceUpdate(long status, IJYEventInfo *eventInfo);
	void ReceivedDeviceCriticalError(long status, IJYEventInfo *eventInfo);
	void WaitForMono();
	ExperimentParameters GetExperimentParameters();
	int GetExperimentProgressIndex(); 
	void AddNewT(int T);
	void PostMessageToUI(UINT message, CString logMessage);
	void EnableExpSettDlg();
	void EnableExpFlowDlg();
	void DisableConnDlg();
	void DisableExpSettDlg();
	bool m_bMeasurementStarted = false;
	bool m_isCCDDataReady = false;
	bool m_isPLCConfirmedOff = false;
	bool m_isMonoInitialised;
	double m_currT = 294.0;

	void StartTimer(UINT_PTR nIDEvent, int _sec);
	void StopTimer(UINT_PTR nIDEvent);
	afx_msg void OnTimer(UINT_PTR nIDEvent); // Ensure this is in the Message Map

	void SetExpProgress();
	void RepeatPreviousT();
	BOOL
		isExitEnabled = FALSE,
		m_isNextExpRefused = FALSE,
		m_isExperimentStarted = FALSE,
		CanExit();

protected:
	CString m_monoArray[10][2];
	long m_gainCCD[3], m_ADCCCD[3];
	
	bool m_bDetectorInitialized;
//	bool m_bDetectorForceInit;
	CComPtr<IJYMonoReqd> m_jyMono;
	CComPtr<CJYDeviceSink> m_sinkPtrMono;
	CComPtr<IJYCCDReqd> m_jyCCD;
	CComPtr<CJYDeviceSink> m_sinkPtrCCD;
	CComPtr<IJYConfigBrowerInterface> m_pConfigBrowser;

	CComPtr<IJYDataObject> m_AcqDataObj;    // Holds data gathered by acquisition to be accessed by the "Save Data" Button


	HICON m_hIcon;
	CiHR320DlgAutoProxy* m_pAutoProxy;

	CTabCtrl m_tab;
	CiHR320ConnectivityDlg m_connectivityDlg;
	CiHR320SettingsDlg m_settingsDlg;
	CiHR320FlowDlg m_flowDlg;

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
	afx_msg LRESULT OnUpdateSystemStatus(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnUpdateSystemEvent(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnPutLog(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnMonoLogMessage(WPARAM wParam, LPARAM lParam);
	void EnableDlg(CWnd * pTargetDlg, BOOL bEnable);
//---------------------------------------------SDK--------------------------------------------------
	void LoadMonos();
	void LoadCCDs();
	BOOL ConnectAndInitCCD();

	BOOL ConnectAndInitMono();


	DECLARE_MESSAGE_MAP()

public:
	jyUnits eUnits;
	VARIANT vUnits;
	CString sUnits;
	void GetGratings();
	void SetMonoDG(int grating);
	void SetAT(double newAT);
	void MonoMoveTo(double newPos);
	void GetSlits();
	void SetSlits(double newSlits);
	void SetMirror();
	std::array<BOOL, 2> ConnectMonoAndCCD();
	HRESULT DoAcquisition(bool shutterOpen = true);
	void SetCCDParams();

};
