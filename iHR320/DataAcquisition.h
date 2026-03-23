#pragma once
class CiHR320Dlg;
struct ExperimentParameters;

struct AcqParams {
	int slits = default_Slits;
	int AT = default_MaxAT;
};

bool TakeSpectrum(CiHR320Dlg* pUI, CString T);

class AcquisitionParameters
{
public:
	AcquisitionParameters();
	~AcquisitionParameters();
	AcqParams currAcqParams;
	int paramCount = 0;
	AcqParams AdjustAcqParam(AcqParams oldAcqParams, double factor);
};
