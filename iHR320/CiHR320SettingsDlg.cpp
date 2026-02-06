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
	DDX_Control(pDX, IDC_SLIDER_DG_POSITIONS, m_sliderDGrangeNo);

	DDX_Control(pDX, IDC_NUMBER_ACQ, m_NA);
	DDX_Control(pDX, IDC_NEW_T, m_VSListBox_T.m_newT);
	DDX_Control(pDX, IDC_SPIN_NEW_T, m_VSListBox_T.m_mod_new_T);
	DDX_Control(pDX, IDC_TEMPERATURE_LIST_BOX, m_VSListBox_T);
	DDX_Control(pDX, IDC_MEASURE_FROM, m_StartWL);
}


BEGIN_MESSAGE_MAP(CiHR320SettingsDlg, CDialogEx)
	ON_EN_KILLFOCUS(IDC_NEW_T, &CiHR320SettingsDlg::OnEnKillfocus_newT)
	ON_BN_CLICKED(IDC_BUTTON_DEFAULT_T, &CiHR320SettingsDlg::OnBnClickedButtonDefaultT)
	ON_BN_CLICKED(IDC_BUTTON_VALIDATE_T, &CiHR320SettingsDlg::OnBnClickedButtonValidateT)
END_MESSAGE_MAP()


BOOL CiHR320SettingsDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

//	m_VSListBox_T
//	Populate Diffraction grating list 
	m_ListBoxDG.AddString(_T("Grating 1"));
	m_ListBoxDG.AddString(_T("Grating 2"));
	m_ListBoxDG.AddString(_T("Grating 3"));

//	Specifying starting wavelength slider
	m_sliderStartWL.SetRange(200, 600);
	m_sliderStartWL.SetTicFreq(10);
	m_sliderStartWL.SetPos(340);

//	Specifying number of DG "windows" slider
	m_sliderDGrangeNo.SetRange(1, 5);
	m_sliderDGrangeNo.SetTicFreq(1);
	m_sliderDGrangeNo.SetPos(3);

	m_NA.SetWindowText(_T("4"));
	m_VSListBox_T.m_newT.SetWindowTextW(_T("300"));
	
	return TRUE;

}



void CiHR320SettingsDlg::OnBnClickedButtonDefaultT()
{
	for (const auto& T : DefaultTs)
	{
		m_VSListBox_T.AddItem(T);
	}
}


void CiHR320SettingsDlg::OnBnClickedButtonValidateT()
{
	m_VSListBox_T.SortT(FALSE);
}

void CiHR320SettingsDlg::OnEnKillfocus_newT()
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
