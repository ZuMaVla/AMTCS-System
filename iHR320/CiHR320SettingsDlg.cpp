// CiHR320SettingsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "CiHR320SettingsDlg.h"
#include "DefaultExperimentSettings.h"
#include "afxdialogex.h"
#include "afxwin.h"
#include "Resource.h"
#include <string>


// CiHR320SettingsDlg dialog

IMPLEMENT_DYNAMIC(CiHR320SettingsDlg, CDialogEx)

CiHR320SettingsDlg::CiHR320SettingsDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_EXPERIMENT_SETTINGS_DLG, pParent)
{

}

CiHR320SettingsDlg::~CiHR320SettingsDlg()
{
}

void CiHR320SettingsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_DG, m_ListBoxDG);
	DDX_Control(pDX, IDC_SLIDER_START_WL, m_sliderStartWL);
	DDX_Control(pDX, IDC_SLIDER_DG_POSITIONS, m_sliderDGRangeNo);

	DDX_Control(pDX, IDC_NUMBER_ACQ, m_NA);
	DDX_Control(pDX, IDC_NEW_T, m_VSListBox_T.m_newT);
	DDX_Control(pDX, IDC_SPIN_NEW_T, m_VSListBox_T.m_mod_new_T);
	DDX_Control(pDX, IDC_TEMPERATURE_LIST_BOX, m_VSListBox_T);
	DDX_Control(pDX, IDC_MEASURE_FROM, m_StartWL);
	DDX_Control(pDX, IDC_CHECK_COSMIC_RAYS, m_isCRRemoval);
	DDX_Control(pDX, IDC_SLITS, m_Slits);
}


BEGIN_MESSAGE_MAP(CiHR320SettingsDlg, CDialogEx)
	ON_EN_KILLFOCUS(IDC_NEW_T, &CiHR320SettingsDlg::OnNewTChanged)
	ON_BN_CLICKED(IDC_BUTTON_DEFAULT_T, &CiHR320SettingsDlg::OnBnClickedButtonDefaultT)
	ON_BN_CLICKED(IDC_BUTTON_VALIDATE_T, &CiHR320SettingsDlg::OnBnClickedButtonValidateT)
	ON_NOTIFY(TRBN_THUMBPOSCHANGING, IDC_SLIDER_START_WL, &CiHR320SettingsDlg::OnStartWLSliderMoving)
	ON_EN_KILLFOCUS(IDC_NUMBER_ACQ, &CiHR320SettingsDlg::OnNAChanged)
	ON_EN_KILLFOCUS(IDC_SLITS, &CiHR320SettingsDlg::OnSlitsChanged)
	ON_EN_KILLFOCUS(IDC_MEASURE_FROM, &CiHR320SettingsDlg::OnStartWLChanged)
END_MESSAGE_MAP()


BOOL CiHR320SettingsDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	CString text;

//	m_VSListBox_T
//	Populate Diffraction grating list 
	m_ListBoxDG.AddString(_T("Grating 1"));
	m_ListBoxDG.AddString(_T("Grating 2"));
	m_ListBoxDG.AddString(_T("Grating 3"));
	m_ListBoxDG.SetCurSel(0);

//	Specifying starting wavelength slider
	m_sliderStartWL.SetRange(ExtremeStartWL[0], ExtremeStartWL[1]);
	m_sliderStartWL.SetTicFreq(10);
	m_sliderStartWL.SetPos(default_StartWL);
	m_sliderStartWL.ModifyStyle(0, TBS_NOTIFYBEFOREMOVE);

	text.Format(L"%d", default_StartWL); 
	m_StartWL.SetWindowTextW(text);

//	Specifying number of DG "windows" slider
	m_sliderDGRangeNo.SetRange(ExtremeDGRangeNo[0], ExtremeDGRangeNo[1]);
	m_sliderDGRangeNo.SetTicFreq(1);
	m_sliderDGRangeNo.SetPos(default_DGRangeNo);


	text.Format(L"%d", default_NA);
	m_NA.SetWindowTextW(text);

	text.Format(L"%d", default_newT);
	m_VSListBox_T.m_newT.SetWindowTextW(text);

	text.Format(L"%d", default_Slits);
	m_Slits.SetWindowTextW(text);

	m_isCRRemoval.SetCheck(BST_CHECKED);
	
	return TRUE;

}



void CiHR320SettingsDlg::OnBnClickedButtonDefaultT()
{
	CString s;
	for (const auto& T : DefaultTs)
	{
		s = T.c_str();
		m_VSListBox_T.AddItem(s);
	}
}


void CiHR320SettingsDlg::OnBnClickedButtonValidateT()
{
	m_VSListBox_T.SortT(FALSE);
}

void CiHR320SettingsDlg::OnNewTChanged()
{
	CString text;
	m_VSListBox_T.m_newT.GetWindowText(text);

	int value = _ttoi(text);

	if (value < ExtremeTs[0] || value > ExtremeTs[1])
	{
		CString msg;
		msg.Format(_T("Value must be between %d and %d"), ExtremeTs[0], ExtremeTs[1]);
		AfxMessageBox(msg);
		m_VSListBox_T.m_newT.SetFocus();
	}
}




void CiHR320SettingsDlg::OnNAChanged()
{
	CString msg;
	BOOL success = FALSE;
	int value = GetDlgItemInt(IDC_NUMBER_ACQ, &success, TRUE);

	if (value < ExtremeNA[0] || value > ExtremeNA[1]) {
		msg.Format(_T("Value must be between %d and %d"), ExtremeNA[0], ExtremeNA[1]);
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
}


void CiHR320SettingsDlg::OnSlitsChanged()
{
	CString msg;
	BOOL success = FALSE;
	int value = GetDlgItemInt(IDC_SLITS, &success, TRUE);

	if (value < ExtremeSlits[0] || value > ExtremeSlits[1]) {
		msg.Format(_T("Value must be between %d and %d"), ExtremeSlits[0], ExtremeSlits[1]);
		AfxMessageBox(msg);
		m_Slits.SetFocus();
		return;
	}
}

void CiHR320SettingsDlg::OnStartWLSliderMoving(
	NMHDR *pNMHDR, LRESULT *pResult)
{
	NMTRBTHUMBPOSCHANGING* pNMTPC = reinterpret_cast<NMTRBTHUMBPOSCHANGING*>(pNMHDR);

	int continuousPos = pNMTPC->dwPos;   // <-- This is the thumb position
//	int tick = m_sliderStartWL.GetTic(2) - m_sliderStartWL.GetTic(1);
	int tick = 10;
	int numberTicks = int((continuousPos + tick / 2) / tick);
	int snappedPos = numberTicks*tick;
//	pNMTPC->dwPos = snappedPos;
	m_sliderStartWL.SetPos(snappedPos);
	CString newStartWL;
	newStartWL.Format(L"%d", snappedPos); // convert int → wide string
	m_StartWL.SetWindowTextW(newStartWL);
	
	*pResult = 0;
}

void CiHR320SettingsDlg::OnStartWLChanged()
{
	CString msg;
	BOOL success = FALSE;
	int value = GetDlgItemInt(IDC_MEASURE_FROM, &success, TRUE);

	if (value < ExtremeStartWL[0] || value > ExtremeStartWL[1]) {
		msg.Format(_T("Value must be between %d and %d"), ExtremeStartWL[0], ExtremeStartWL[1]);
		AfxMessageBox(msg);
		m_StartWL.SetFocus();
		return;
	}
	m_sliderStartWL.SetPos(value);

}
