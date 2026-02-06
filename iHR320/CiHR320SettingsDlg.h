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
	CSliderCtrl m_sliderDGrangeNo;						// Slider for varying the number of ranges of a chosen diffraction grating
	CEdit m_NA;											// Number of acquisitions (averagings) per spectrum
	CMyVSListBox m_VSListBox_T;							// List of temperatures at which spectra are to be measured 
	afx_msg void OnBnClickedButtonDefaultT();
	afx_msg void OnBnClickedButtonValidateT();
	afx_msg void OnEnKillfocus_newT();
};
