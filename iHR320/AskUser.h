#pragma once
#include "afxwin.h"


// CAskUser dialog

class CAskUser : public CDialogEx
{
	DECLARE_DYNAMIC(CAskUser)

public:
	CAskUser(CWnd* pParent = NULL);   // standard constructor
	virtual ~CAskUser();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ASK_USER };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnClose();
	virtual void OnOK();
	virtual void OnCancel();
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
	CStatic m_question;
	CString s_question;
};
