// CiHR320FlowDlg.cpp : implementation file
//

#include "stdafx.h"
//#include "iHR320.h"
#include "CiHR320FlowDlg.h"
#include "afxdialogex.h"
#include "Resource.h"
#include "iHR320Dlg.h"
#include "TCPtoRPi.h"


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
	DDX_Control(pDX, IDC_EXP_FLOW_LOGS, m_ExpFlowLogs);
	DDX_Control(pDX, IDC_EXP_PROGRESS, m_expProgressBar);
	DDX_Control(pDX, IDC_REPEAT, m_repeatPreviousTBtn);
	DDX_Control(pDX, IDC_PAUSE, m_pauseResumeBtn);
	DDX_Control(pDX, IDC_CANCEL, m_cancelExp);
	DDX_Control(pDX, IDC_ADD_T, m_addNewTBtn);
	DDX_Control(pDX, IDC_NEW_T, m_newAddT);
	DDX_Control(pDX, IDC_CURRENT_TARGET, m_currTargetT);
}


BEGIN_MESSAGE_MAP(CiHR320FlowDlg, CDialogEx)
	ON_BN_CLICKED(IDC_REPEAT, &CiHR320FlowDlg::OnBnClickedRepeatT)
	ON_BN_CLICKED(IDC_PAUSE, &CiHR320FlowDlg::OnBnClickedPause)
	ON_BN_CLICKED(IDC_CANCEL, &CiHR320FlowDlg::OnClickedCancelExp)
	ON_BN_CLICKED(IDC_ADD_T, &CiHR320FlowDlg::OnClickedAddNewT)
	ON_EN_KILLFOCUS(IDC_NEW_T, &CiHR320FlowDlg::OnKillfocusNewT)
END_MESSAGE_MAP()

BOOL CiHR320FlowDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	m_ExpFlowLogs.EnableBrowseButton(FALSE);
	

	return TRUE;
}

// CiHR320FlowDlg message handlers


void CiHR320FlowDlg::OnBnClickedRepeatT()
{
	m_mainWnd->RepeatPreviousT();
}


void CiHR320FlowDlg::OnBnClickedPause()
{
	CString actionStr;
	m_pauseResumeBtn.GetWindowTextW(actionStr);
	if (actionStr == _T("Pause indefinitely")) {
		m_ExpFlowLogs.AddItem(_T("User requested to pause experiment."));
		if (!(SendTCPMessage(m_mainWnd, ip_PLC, port_PLC, "PAUSE"))) {
			AfxMessageBox(_T("Connection failed"));
			return;
		}
		m_nextUserAction = _T("Continue");
	}
	else {
		m_ExpFlowLogs.AddItem(_T("User requested to continue experiment."));
		if (!(SendTCPMessage(m_mainWnd, ip_PLC, port_PLC, "CONTINUE"))) {
			AfxMessageBox(_T("Connection failed"));
			return;
		}
		m_nextUserAction = _T("Pause indefinitely");
	}
}


void CiHR320FlowDlg::OnClickedCancelExp()
{
	m_mainWnd->m_askUser.s_question = L"Are you sure you want to cancel current experiment?";
	if (m_mainWnd->m_askUser.DoModal() == IDOK) {
		if (!(SendTCPMessage(m_mainWnd, ip_PLC, port_PLC, "CANCEL"))) {
			AfxMessageBox(_T("Connection failed"));
			return;
		}
	}
}


void CiHR320FlowDlg::OnClickedAddNewT()
{
	BOOL success = FALSE;
	int T = GetDlgItemInt(IDC_NEW_T, &success, TRUE);  // Retrieving temperature

	if (!success) {
		return;
	}
	m_mainWnd->AddNewT(T);
}


void CiHR320FlowDlg::OnKillfocusNewT()
{
	CString text;
	m_newAddT.GetWindowTextW(text);

	int value = _ttoi(text);

	if (value < extreme_Ts[0] || value > extreme_Ts[1])
	{
		CString msg;
		msg.Format(_T("Value must be between %d and %d [K]"), extreme_Ts[0], extreme_Ts[1]);
		AfxMessageBox(msg);
		m_newAddT.SetFocus();
	}

}
