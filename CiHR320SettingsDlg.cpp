// CiHR320SettingsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "iHR320.h"
#include "CiHR320SettingsDlg.h"
#include "afxdialogex.h"
#include "Resource.h"


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

	DDX_Control(pDX, IDC_EDIT2, m_NA);
}


BEGIN_MESSAGE_MAP(CiHR320SettingsDlg, CDialogEx)
END_MESSAGE_MAP()


BOOL CiHR320SettingsDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();


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



	return TRUE;

}
