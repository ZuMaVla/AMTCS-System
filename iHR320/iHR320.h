
// iHR320.h : main header file for the PROJECT_NAME application
//

#pragma once


#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "Resource.h"		// main symbols
#include <atlbase.h>
#include <atlcom.h>

extern bool g_isPLCOnAtStart;

// CiHR320App:

class CiHR320App : public CWinApp
{
public:
	CiHR320App();
// Overrides
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
// Implementation

	DECLARE_MESSAGE_MAP()
private:
	BOOL m_bATLInited;
	BOOL InitATL();
};

extern CiHR320App theApp;