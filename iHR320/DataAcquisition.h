#pragma once
class CiHR320Dlg;
struct ExperimentParameters;


bool TakeSpectrum(CiHR320Dlg* pUI, CString T);

class AcquisitionParameters
{
public:
	AcquisitionParameters();
	~AcquisitionParameters();
	int slits = -1;			// Indication of not yet initialised value
	int AT = -1;			// Indication of not yet initialised value (acquisition time)
	int paramCount = 0;
	bool AdjustAcqParam(int maxAT, int maxSlits, double factor);
};
