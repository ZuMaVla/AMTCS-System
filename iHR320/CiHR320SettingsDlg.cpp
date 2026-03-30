// CiHR320SettingsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "CiHR320SettingsDlg.h"
#include "DefaultExperimentSettings.h"
#include "afxdialogex.h"
#include "afxwin.h"
#include "Resource.h"
#include <string>
#include <shlobj.h>
#include <json.hpp>
#include <iostream>
#include "TCPtoRPi.h"
#include "CiHR320ConnectivityDlg.h"
#include "iHR320Dlg.h"
#include "DataAcquisition.h"
#include <set>

// CiHR320SettingsDlg dialog

IMPLEMENT_DYNAMIC(CiHR320SettingsDlg, CDialogEx)

CiHR320SettingsDlg::CiHR320SettingsDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_EXPERIMENT_SETTINGS_DLG, pParent)
{
}

CiHR320SettingsDlg::~CiHR320SettingsDlg()
{
}

void CiHR320SettingsDlg::SetMainWnd(CiHR320Dlg * main)
{
	m_mainWnd = main;
}

ExperimentParameters CiHR320SettingsDlg::GetExperimentParameters()
{
	return experimentState.getExpParams();
}

void CiHR320SettingsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_DG, m_ListBoxDG);
	DDX_Control(pDX, IDC_SLIDER_START_WL, m_sliderStartWL);
	DDX_Control(pDX, IDC_SLIDER_DG_POSITIONS, m_sliderDGRangeNo);
	DDX_Control(pDX, IDC_Acq, m_acquisBtnTemp);
	DDX_Control(pDX, IDC_NUMBER_ACQ, m_NA);
	DDX_Control(pDX, IDC_NEW_T, m_VSListBox_T.m_newT);
	DDX_Control(pDX, IDC_SPIN_NEW_T, m_VSListBox_T.m_mod_new_T);
	DDX_Control(pDX, IDC_TEMPERATURE_LIST_BOX, m_VSListBox_T);
	DDX_Control(pDX, IDC_MEASURE_FROM, m_StartWL);
	DDX_Control(pDX, IDC_CHECK_COSMIC_RAYS, m_isCRRemoval);
	DDX_Control(pDX, IDC_SLITS, m_Slits);
	DDX_Control(pDX, IDC_ACQUISITION_TIME_MAX, m_maxAT);
	DDX_Control(pDX, IDC_SAVE_FOLDER, m_workDir);
	DDX_Control(pDX, IDC_SAMPLE_CODE, m_sampleCode);
	DDX_Control(pDX, IDC_START, m_startExpBtn);
}


BEGIN_MESSAGE_MAP(CiHR320SettingsDlg, CDialogEx)
	ON_EN_KILLFOCUS(IDC_NEW_T, &CiHR320SettingsDlg::OnNewTChanged)
	ON_BN_CLICKED(IDC_BUTTON_DEFAULT_T, &CiHR320SettingsDlg::OnBnClickedButtonDefaultT)
	ON_BN_CLICKED(IDC_BUTTON_VALIDATE_T, &CiHR320SettingsDlg::OnBnClickedClearListT)
	ON_NOTIFY(TRBN_THUMBPOSCHANGING, IDC_SLIDER_START_WL, &CiHR320SettingsDlg::OnStartWLSliderMoving)
	ON_EN_KILLFOCUS(IDC_NUMBER_ACQ, &CiHR320SettingsDlg::OnNAChanged)
	ON_EN_KILLFOCUS(IDC_SLITS, &CiHR320SettingsDlg::OnSlitsChanged)
	ON_EN_KILLFOCUS(IDC_MEASURE_FROM, &CiHR320SettingsDlg::OnStartWLChanged)
	ON_EN_KILLFOCUS(IDC_ACQUISITION_TIME_MAX, &CiHR320SettingsDlg::OnMaxATChanged)
	ON_EN_KILLFOCUS(IDC_SAVE_FOLDER, &CiHR320SettingsDlg::OnWorkDirChanged)
	ON_BN_CLICKED(IDC_START, &CiHR320SettingsDlg::OnBnClickedStart)
	ON_LBN_SELCHANGE(IDC_LIST_DG, &CiHR320SettingsDlg::OnMonoDGChanged)
	ON_BN_CLICKED(IDC_Acq, &CiHR320SettingsDlg::OnBnClickedAcq)

	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER_DG_POSITIONS, &CiHR320SettingsDlg::OnDGRangeNoChanged)
	ON_EN_CHANGE(IDC_SAMPLE_CODE, &CiHR320SettingsDlg::OnSampleCodeChanged)
END_MESSAGE_MAP()


BOOL CiHR320SettingsDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	CString text;
	ExperimentConfiguration();
	SetExperimentParameters();
	
	CStdioFile file;
	if (file.Open(L"workDir.txt", CFile::modeRead | CFile::typeText)) {
		file.ReadString(text);
		file.Close();
	}
	else {
		text = L"";
	}
	m_workDir.SetWindowTextW(text);

	
	return TRUE;

}

void CiHR320SettingsDlg::OnBnClickedAcq()			// For testing acquisition
{
	TakeSpectrum(m_mainWnd, L"300");
}


void CiHR320SettingsDlg::OnBnClickedButtonDefaultT()
{
	m_VSListBox_T.RemoveAll();
	m_VSListBox_T.AddAll(default_Ts);
}


void CiHR320SettingsDlg::OnBnClickedClearListT()
{
	m_VSListBox_T.RemoveAll();
	m_VSListBox_T.m_newT.SetFocus();
}

void CiHR320SettingsDlg::ExperimentConfiguration()
{
	CString text;

	//	Populate Diffraction grating list 
	m_ListBoxDG.AddString(_T("Grating 1"));
	m_ListBoxDG.AddString(_T("Grating 2"));
	m_ListBoxDG.AddString(_T("Grating 3"));

	//	Specifying starting wavelength limits
	m_sliderStartWL.SetRange(extreme_StartWL[0], extreme_StartWL[1]);
	m_sliderStartWL.SetTicFreq(10);
	m_sliderStartWL.ModifyStyle(0, TBS_NOTIFYBEFOREMOVE);

	//	Specifying number of DG "windows" slider
	m_sliderDGRangeNo.SetRange(extreme_DGRangeNo[0], extreme_DGRangeNo[1]);
	m_sliderDGRangeNo.SetTicFreq(1);

	//	Specify default new temperature
	text.Format(L"%d", default_newT);
	m_VSListBox_T.m_newT.SetWindowTextW(text);

}

ExperimentParameters CiHR320SettingsDlg::CollectExperimentParameters()
{
	ExperimentParameters expParams;								// Experiment parameters container to be populated from UI
	CString text;

	// 1. Retrieve Sample Code
	m_sampleCode.GetWindowTextW(text);
	expParams.sampleCode = std::string(CT2A(text, CP_UTF8));

	// 2. Retrieve current temperature list
	expParams.Ts = m_VSListBox_T.GetAllItemTs();

	// 3. Retrieve diffraction grating selection
	expParams.DG = m_ListBoxDG.GetCurSel();

	// 4. Retrieve starting wavelength
	m_StartWL.GetWindowTextW(text);
	expParams.StartWL = _ttoi(text);

	// 5. Retrieve number of DG ranges (spectral windows) 
	expParams.DGRangeNo = m_sliderDGRangeNo.GetPos();

	// 6. Retrieve number of acquisitions per scan 
	m_NA.GetWindowTextW(text);
	expParams.NA = _ttoi(text);

	// 7. Retrieve max slits width 
	m_Slits.GetWindowTextW(text);
	expParams.slits = _ttoi(text);

	// 8. Retrieve max acquisition time
	m_maxAT.GetWindowTextW(text);
	expParams.maxAT = _ttoi(text);

	// 9. Retrieve cosmic ray removal setting
	expParams.isCRRemoval = (m_isCRRemoval.GetCheck() == BST_CHECKED);

	return expParams;
}

void CiHR320SettingsDlg::SetExperimentParameters()
{
	ExperimentParameters expParams = experimentState.getExpParams();

	// 1. Specify sample code
	CString text(expParams.sampleCode.c_str());
	m_sampleCode.SetWindowTextW(text);

	// 2. Set temperature list
	m_VSListBox_T.RemoveAll();
	m_VSListBox_T.AddAll(expParams.Ts);

	// 3. Select diffraction grating  
	m_ListBoxDG.SetCurSel(expParams.DG);

	// 4. Specify starting wavelength
	m_sliderStartWL.SetPos(expParams.StartWL);
	text.Format(L"%d", expParams.StartWL);
	m_StartWL.SetWindowTextW(text);

	// 5. Specify number of DG ranges (spectral windows)
	m_sliderDGRangeNo.SetPos(expParams.DGRangeNo);

	// 6. Specify number of acquisitions per scan
	text.Format(L"%d", expParams.NA);
	m_NA.SetWindowTextW(text);

	// 7. Specify default slits' width
	text.Format(L"%d", expParams.slits);
	m_Slits.SetWindowTextW(text);

	// 8. Specify max acquisition time
	text.Format(L"%d", expParams.maxAT);
	m_maxAT.SetWindowTextW(text);

	// 9. Set cosmic ray check box state
	if (expParams.isCRRemoval)
	{
		m_isCRRemoval.SetCheck(BST_CHECKED);
	}
	else
	{
		m_isCRRemoval.SetCheck(BST_UNCHECKED);
	}

}

void CiHR320SettingsDlg::OnNewTChanged()
{
	CString text;
	m_VSListBox_T.m_newT.GetWindowText(text);

	int value = _ttoi(text);

	if (value < extreme_Ts[0] || value > extreme_Ts[1])
	{
		CString msg;
		msg.Format(_T("Value must be between %d and %d [K]"), extreme_Ts[0], extreme_Ts[1]);
		AfxMessageBox(msg);
		m_VSListBox_T.m_newT.SetFocus();
	}
}

void CiHR320SettingsDlg::OnNAChanged()
{
	CString msg;
	BOOL success = FALSE;
	int value = GetDlgItemInt(IDC_NUMBER_ACQ, &success, TRUE);
	ExperimentParameters temp = experimentState.getExpParams();
	
	if (value < extreme_NA[0] || value > extreme_NA[1]) {
		msg.Format(_T("Value must be between %d and %d"), extreme_NA[0], extreme_NA[1]);
		AfxMessageBox(msg);
		m_NA.SetFocus();
		return;
	}
	if (value < 3) {
		m_isCRRemoval.SetCheck(BST_UNCHECKED);
		m_isCRRemoval.EnableWindow(FALSE);
	}
	else {
		m_isCRRemoval.EnableWindow(TRUE);
	}
	temp.NA = value;
	experimentState.setExpParams(temp);
}


void CiHR320SettingsDlg::OnSlitsChanged()
{
	CString msg;
	BOOL success = FALSE;
	int value = GetDlgItemInt(IDC_SLITS, &success, TRUE);
	ExperimentParameters temp = experimentState.getExpParams();

	if (value < extreme_Slits[0] || value > extreme_Slits[1]) {
		msg.Format(_T("Value must be between %d and %d [µm]"), extreme_Slits[0], extreme_Slits[1]);
		AfxMessageBox(msg);
		m_Slits.SetFocus();
		return;
	}
	m_mainWnd->SetSlits(value/1000.0);		// Convert µm (UI) -> mm (CCD)
	m_mainWnd->m_acqParams.slits = value;
	temp.slits = value;
	experimentState.setExpParams(temp);
}

void CiHR320SettingsDlg::OnStartWLSliderMoving(
	NMHDR *pNMHDR, LRESULT *pResult)
{
	NMTRBTHUMBPOSCHANGING* pNMTPC = reinterpret_cast<NMTRBTHUMBPOSCHANGING*>(pNMHDR);
	ExperimentParameters temp = experimentState.getExpParams();

	int continuousPos = pNMTPC->dwPos;   // <-- This is the thumb position
	int tick = 10;
	int numberTicks = int((continuousPos + tick / 2) / tick);
	int snappedPos = numberTicks*tick;
	m_sliderStartWL.SetPos(snappedPos);
	CString newStartWL;
	newStartWL.Format(L"%d", snappedPos); // convert int → wide string
	m_StartWL.SetWindowTextW(newStartWL);
	
	*pResult = 0;
	temp.StartWL = snappedPos;
	experimentState.setExpParams(temp);
}

void CiHR320SettingsDlg::OnStartWLChanged()
{
	CString msg;
	BOOL success = FALSE;
	int value = GetDlgItemInt(IDC_MEASURE_FROM, &success, TRUE);
	ExperimentParameters temp = experimentState.getExpParams();

	if (value < extreme_StartWL[0] || value > extreme_StartWL[1]) {
		msg.Format(_T("Value must be between %d and %d [nm]"), extreme_StartWL[0], extreme_StartWL[1]);
		AfxMessageBox(msg);
		m_StartWL.SetFocus();
		return;
	}
	m_sliderStartWL.SetPos(value);
	temp.StartWL = value;
	experimentState.setExpParams(temp);
}


void CiHR320SettingsDlg::OnMaxATChanged()
{
	CString msg;
	BOOL success = FALSE;
	int value = GetDlgItemInt(IDC_ACQUISITION_TIME_MAX, &success, TRUE);
	ExperimentParameters temp = experimentState.getExpParams();

	if (value < extreme_AT[0] || value > extreme_AT[1]) {
		msg.Format(_T("Value must be between %d and %d [ms]"), extreme_AT[0], extreme_AT[1]);
		AfxMessageBox(msg);
		m_StartWL.SetFocus();
		return;
	}
	m_mainWnd->SetAT(value/1000.0);
	m_mainWnd->m_acqParams.AT = value;
	temp.maxAT = value;
	experimentState.setExpParams(temp);
}


void CiHR320SettingsDlg::OnWorkDirChanged()
{
	CString path;
	m_workDir.GetWindowTextW(path);
	
	if (path.IsEmpty()) {													// "Is the path empty?"-check 
		AfxMessageBox(L"Path cannot be empty!");
		m_workDir.SetFocus();
		return; 
	}
	
	DWORD dwAttrib = GetFileAttributesW(path);
	bool isExistsAsDir = (dwAttrib != INVALID_FILE_ATTRIBUTES &&			// To flag if path to existing directory
		(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));

	if (!isExistsAsDir)														// Otherwise... 
	{
		int result = SHCreateDirectoryEx(NULL, path, NULL);
		if (result != ERROR_SUCCESS && result != ERROR_ALREADY_EXISTS)		// Check if dir can be created
		{
			AfxMessageBox(L"The folder does not exist and could not be created.\nPlease check the path syntax or drive permissions.");
			m_workDir.SetFocus();
			return;
		}
	}

	CString testFilePath = path;
	if (testFilePath.Right(1) != L"\\") testFilePath += L"\\";				// Ensuring path is ending with "\"
	testFilePath += L"write-access-test";

	CFile testFile;
	if (testFile.Open(testFilePath, CFile::modeCreate | CFile::modeWrite))	// Test for write access 
	{
		testFile.Close();
		CFile::Remove(testFilePath);										// Cleanup

									 
		CStdioFile file;
		if (file.Open(L"workDir.txt", CFile::modeCreate | CFile::modeWrite | CFile::typeText)) {
			file.WriteString(path);											// Success: Save the configuration
			file.Close();
		}
	}
	else
	{
		AfxMessageBox(L"Access Denied: You do not have permission to write files to this folder.");
		m_workDir.SetFocus();												// Path exists/created, but OS blocked file creation
		return;
	}
}

void CiHR320SettingsDlg::OnBnClickedStart()
{
	if (!m_mainWnd->m_isExperimentStarted) {		// Check if not yet in experiment
		if (m_VSListBox_T.GetCount() > 0) {
			m_VSListBox_T.SortT(FALSE);				// Sort descending order
			if (m_VSListBox_T.m_isHT) {
				m_mainWnd->m_askUser.s_question = L"Are you sure you want add over 300K temperatures?";
				if (m_mainWnd->m_askUser.DoModal() != IDOK)
					return;
			}
			if (abs(m_VSListBox_T.getLast() - m_mainWnd->m_currT) < abs(m_VSListBox_T.getFirst() - m_mainWnd->m_currT))
			{
				m_VSListBox_T.SortT(TRUE);			// Reverse sorting if not optimal order
			}
			experimentState.setExpParams(CollectExperimentParameters());
			experimentState.experimentProgressIndex = -1;
			experimentState.experimentLength = experimentState.getExpParams().Ts.size();
			m_mainWnd->DisableExpSettDlg();
			m_mainWnd->m_acqParams.slits = experimentState.getExpParams().slits;
			m_mainWnd->m_acqParams.AT = experimentState.getExpParams().maxAT;
		}
		else {
			AfxMessageBox(_T("At least one temperature is required to start an experiment!"));	// If T list is empty
			return;
		}
		m_mainWnd->m_flowDlg.m_ExpFlowLogs.AddItem(_T("Experiment started..."));				// Put log about starting experiment
	}
	std::string msg;
	msg = experimentState.serialiseState();
	std::cout << "[UI-APP] Serialised experiment state: " << msg << "\n";
	if (!(SendTCPMessage(m_mainWnd, ip_PLC, port_PLC, "INIT " + msg))) {
		AfxMessageBox(_T("Connection failed"));
		m_mainWnd->StopTimer(TIMER_EXP_SENT);
	}
	m_mainWnd->DisableExpSettDlg();
	SetThreadExecutionState(ES_CONTINUOUS | ES_SYSTEM_REQUIRED);				// prevents system from sleeping
	Sleep(2000);
	m_mainWnd->SelectTab(2);
}




void CiHR320SettingsDlg::OnMonoDGChanged()
{
	int index = m_ListBoxDG.GetCurSel();
	ExperimentParameters temp = experimentState.getExpParams();

	m_mainWnd->SetMonoDG(index);
	temp.DG = index;
	experimentState.setExpParams(temp);
}


void CiHR320SettingsDlg::OnDGRangeNoChanged(NMHDR *pNMHDR, LRESULT *pResult)
{
	int value = m_sliderDGRangeNo.GetPos();
	ExperimentParameters temp = experimentState.getExpParams();
	temp.DGRangeNo = value;
	experimentState.setExpParams(temp);

	*pResult = 0;
}



void CiHR320SettingsDlg::OnSampleCodeChanged()
{
	CString strText;
	m_sampleCode.GetWindowText(strText);
	ExperimentParameters temp = experimentState.getExpParams();

	CString forbiddenChars = _T("\\/:*?\"<>|");			// Characters to exclude from sample name

	int foundIndex = strText.FindOneOf(forbiddenChars);

	if (foundIndex != -1)
	{
		
		TCHAR badChar = strText.GetAt(foundIndex);		// To show a specific bad character to the user
		CString errorMsg;
		errorMsg.Format(_T("The character '%c' is not allowed in sample codes."), badChar);

		AfxMessageBox(errorMsg, MB_ICONSTOP);
		m_sampleCode.SetFocus();
	}
	CT2A asciiText(strText);
	temp.sampleCode = std::string(asciiText);
	experimentState.setExpParams(temp);
}