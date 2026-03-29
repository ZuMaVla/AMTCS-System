// AskUser.cpp : implementation file
//

#include "stdafx.h"
#include "AskUser.h"
#include "Resource.h"
#include "afxdialogex.h"


// CAskUser dialog

IMPLEMENT_DYNAMIC(CAskUser, CDialogEx)

CAskUser::CAskUser(CWnd* pParent) : CDialogEx(IDD_ASK_USER, pParent)
{
}

CAskUser::~CAskUser()
{
}

void CAskUser::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_MESSAGE_TO_USER, m_question);
}

// CAskUser message handlers
void CAskUser::OnClose()
{
	CDialogEx::OnClose();
}

void CAskUser::OnOK()
{
	CDialogEx::OnOK();
}

void CAskUser::OnCancel()
{
	CDialogEx::OnCancel();
}

BOOL CAskUser::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	m_question.SetWindowTextW(s_question);
	return TRUE;
}

BEGIN_MESSAGE_MAP(CAskUser, CDialogEx)
END_MESSAGE_MAP()


