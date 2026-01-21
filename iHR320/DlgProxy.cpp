
// DlgProxy.cpp : implementation file
//

#include "stdafx.h"
#include "iHR320.h"
#include "DlgProxy.h"
#include "iHR320Dlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CiHR320DlgAutoProxy

IMPLEMENT_DYNCREATE(CiHR320DlgAutoProxy, CCmdTarget)

CiHR320DlgAutoProxy::CiHR320DlgAutoProxy()
{
	EnableAutomation();
	
	// To keep the application running as long as an automation 
	//	object is active, the constructor calls AfxOleLockApp.
	AfxOleLockApp();

	// Get access to the dialog through the application's
	//  main window pointer.  Set the proxy's internal pointer
	//  to point to the dialog, and set the dialog's back pointer to
	//  this proxy.
	ASSERT_VALID(AfxGetApp()->m_pMainWnd);
	if (AfxGetApp()->m_pMainWnd)
	{
		ASSERT_KINDOF(CiHR320Dlg, AfxGetApp()->m_pMainWnd);
		if (AfxGetApp()->m_pMainWnd->IsKindOf(RUNTIME_CLASS(CiHR320Dlg)))
		{
			m_pDialog = reinterpret_cast<CiHR320Dlg*>(AfxGetApp()->m_pMainWnd);
			m_pDialog->m_pAutoProxy = this;
		}
	}
}

CiHR320DlgAutoProxy::~CiHR320DlgAutoProxy()
{
	// To terminate the application when all objects created with
	// 	with automation, the destructor calls AfxOleUnlockApp.
	//  Among other things, this will destroy the main dialog
	if (m_pDialog != NULL)
		m_pDialog->m_pAutoProxy = NULL;
	AfxOleUnlockApp();
}

void CiHR320DlgAutoProxy::OnFinalRelease()
{
	// When the last reference for an automation object is released
	// OnFinalRelease is called.  The base class will automatically
	// deletes the object.  Add additional cleanup required for your
	// object before calling the base class.

	CCmdTarget::OnFinalRelease();
}

BEGIN_MESSAGE_MAP(CiHR320DlgAutoProxy, CCmdTarget)
END_MESSAGE_MAP()

BEGIN_DISPATCH_MAP(CiHR320DlgAutoProxy, CCmdTarget)
END_DISPATCH_MAP()

// Note: we add support for IID_IiHR320 to support typesafe binding
//  from VBA.  This IID must match the GUID that is attached to the 
//  dispinterface in the .IDL file.

// {52AA4D6E-4383-4CFF-90FB-8886361BD9C2}
static const IID IID_IiHR320 =
{ 0x52AA4D6E, 0x4383, 0x4CFF, { 0x90, 0xFB, 0x88, 0x86, 0x36, 0x1B, 0xD9, 0xC2 } };

BEGIN_INTERFACE_MAP(CiHR320DlgAutoProxy, CCmdTarget)
	INTERFACE_PART(CiHR320DlgAutoProxy, IID_IiHR320, Dispatch)
END_INTERFACE_MAP()

// The IMPLEMENT_OLECREATE2 macro is defined in StdAfx.h of this project
// {C0219464-4B1D-4583-B89B-2E65F8439BBB}
IMPLEMENT_OLECREATE2(CiHR320DlgAutoProxy, "iHR320.Application", 0xc0219464, 0x4b1d, 0x4583, 0xb8, 0x9b, 0x2e, 0x65, 0xf8, 0x43, 0x9b, 0xbb)


// CiHR320DlgAutoProxy message handlers
