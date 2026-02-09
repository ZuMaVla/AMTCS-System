
// iHR320.h : main header file for the PROJECT_NAME application
//

#pragma once


#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "Resource.h"		// main symbols

// CiHR320App:
// See iHR320.cpp for the implementation of this class
//

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
};

extern CiHR320App theApp;