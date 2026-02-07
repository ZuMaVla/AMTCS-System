#pragma once
#include "afxwin.h"
#include "afxcmn.h"
#include "MyVSListBox.h"


// CiHR320SettingsDlg dialog

class CiHR320SettingsDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CiHR320SettingsDlg)

public:
	CiHR320SettingsDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CiHR320SettingsDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_EXPERIMENT_SETTINGS_DLG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	CListBox m_ListBoxDG;								// List of diffraction gratings
	CSliderCtrl m_sliderStartWL;						// Slider for varying starting wavelength (for spectra acquisition)
	CEdit m_StartWL;									// Starting wavelength 
	CSliderCtrl m_sliderDGRangeNo;						// Slider for varying the number of ranges of a chosen diffraction grating
	CEdit m_NA;											// Number of acquisitions (averagings) per spectrum
	CMyVSListBox m_VSListBox_T;							// List of temperatures at which spectra are to be measured 
	CButton m_isCRRemoval;								// flag showing whether "cosmic ray" faults are to be dealt with or ignored
	CEdit m_Slits;										// Input slits width (in micrometers)
	afx_msg void OnBnClickedButtonDefaultT();
	afx_msg void OnBnClickedButtonValidateT();
	
	afx_msg void OnStartWLSliderMoving(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNewTChanged();
	afx_msg void OnNAChanged();
	afx_msg void OnSlitsChanged();
	afx_msg void OnStartWLChanged();
};
