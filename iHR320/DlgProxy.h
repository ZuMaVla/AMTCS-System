
// DlgProxy.h: header file
//

#pragma once

class CiHR320Dlg;


// CiHR320DlgAutoProxy command target

class CiHR320DlgAutoProxy : public CCmdTarget
{
	DECLARE_DYNCREATE(CiHR320DlgAutoProxy)

	CiHR320DlgAutoProxy();           // protected constructor used by dynamic creation

// Attributes
public:
	CiHR320Dlg* m_pDialog;

// Operations
public:

// Overrides
	public:
	virtual void OnFinalRelease();

// Implementation
protected:
	virtual ~CiHR320DlgAutoProxy();

	// Generated message map functions

	DECLARE_MESSAGE_MAP()
	DECLARE_OLECREATE(CiHR320DlgAutoProxy)

	// Generated OLE dispatch map functions

	DECLARE_DISPATCH_MAP()
	DECLARE_INTERFACE_MAP()
};

