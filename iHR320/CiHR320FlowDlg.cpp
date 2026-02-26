// CiHR320FlowDlg.cpp : implementation file
//

#include "stdafx.h"
//#include "iHR320.h"
#include "CiHR320FlowDlg.h"
#include "afxdialogex.h"
#include "Resource.h"


// CiHR320FlowDlg dialog

IMPLEMENT_DYNAMIC(CiHR320FlowDlg, CDialogEx)

CiHR320FlowDlg::CiHR320FlowDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_EXPERIMENT_FLOW_DLG, pParent)
{

}

CiHR320FlowDlg::~CiHR320FlowDlg()
{
}

void CiHR320FlowDlg::SetMainWnd(CiHR320Dlg * main)
{
	m_mainWnd = main;
}

void CiHR320FlowDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EXP_FLOW_LOGS, m_ExpFLowLogs);
}


BEGIN_MESSAGE_MAP(CiHR320FlowDlg, CDialogEx)
END_MESSAGE_MAP()

BOOL CiHR320FlowDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
//	m_ExpFLowLogs.Ge
	m_ExpFLowLogs.EnableBrowseButton(FALSE);
	m_ExpFLowLogs.AddItem(_T("Experiment started..."));
	

	return TRUE;
}

// CiHR320FlowDlg message handlers
