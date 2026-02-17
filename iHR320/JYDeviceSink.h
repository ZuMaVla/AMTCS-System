#pragma once
// JYDeviceSink.h: interface for the CJYDeviceSink class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_JYDEVICESINK_H__21E97937_245B_459D_8EB0_30B71038E9FA__INCLUDED_)
#define AFX_JYDEVICESINK_H__21E97937_245B_459D_8EB0_30B71038E9FA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "iHR320Dlg.h"

#define ARBITRARY_SINK 42


#define WM_JY_SYS_DEVICE_MSG_BASE								WM_USER+100
#define WM_JY_SYS_DEVICE_INITIALIZED						WM_JY_SYS_DEVICE_MSG_BASE+1
#define WM_JY_SYS_DEVICE_UPDATE									WM_JY_SYS_DEVICE_MSG_BASE+2
#define WM_JY_SYS_DEVICE_OPERATION_STATUS				WM_JY_SYS_DEVICE_MSG_BASE+3
#define WM_JY_SYS_DEVICE_CRITICAL_ERROR					WM_JY_SYS_DEVICE_MSG_BASE+4


class CJYDeviceSink :
	public IDispEventImpl<ARBITRARY_SINK, CJYDeviceSink, &DIID__IJYDeviceReqdEvents, &LIBID_JYSYSTEMLIBLib, 1, 0 >
{
public:
	CJYDeviceSink(CiHR320Dlg *parentPtr, IJYSystemReqd *eventSource);

	virtual ~CJYDeviceSink();

	void _stdcall OnInitialized(long status, IJYEventInfo *eventInfo);
	void _stdcall OnUpdate(long updateType, IJYEventInfo *eventInfo);
	void _stdcall OnCriticalError(long status, IJYEventInfo *eventInfo);
	void _stdcall OnOperationStatus(long status, IJYEventInfo *eventInfo);

	BEGIN_SINK_MAP(CJYDeviceSink)
		SINK_ENTRY_EX(ARBITRARY_SINK, DIID__IJYDeviceReqdEvents, 1, OnInitialized)
		SINK_ENTRY_EX(ARBITRARY_SINK, DIID__IJYDeviceReqdEvents, 2, OnOperationStatus)
		SINK_ENTRY_EX(ARBITRARY_SINK, DIID__IJYDeviceReqdEvents, 3, OnUpdate)
		SINK_ENTRY_EX(ARBITRARY_SINK, DIID__IJYDeviceReqdEvents, 4, OnCriticalError)
	END_SINK_MAP()

private:
	IJYEventInfoPtr m_eventPtr;
	IJYSystemReqd   *m_eventSourcePtr;
	CiHR320Dlg *m_parentPtr;

};



#endif // !defined(AFX_JYDEVICESINK_H__21E97937_245B_459D_8EB0_30B71038E9FA__INCLUDED_)
