#include "stdafx.h"
#include "DataAcquisition.h"
#include "iHR320Dlg.h"


bool TakeSpectrum(CiHR320Dlg* pUI) {
	pUI->DoAcquisition();
	return true;
};
