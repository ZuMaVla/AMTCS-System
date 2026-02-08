	#pragma once
#include "DefaultExperimentSettings.h"
#include <vector>

struct ExperimentParameters {
	std::string sampleCode = default_SampleCode;
	std::vector<std::string> Ts = default_Ts;
	int StartWL = default_StartWL;
	int DG = default_DG;
	int DGRangeNo = default_DGRangeNo;
	int NA = default_NA;
	int slits = default_Slits;
	int maxAT = default_MaxAT;
	bool isCRRemoval = default_isCRRemoval;
};

class CExperimentState
{
public:
	CExperimentState();
	~CExperimentState();

	std::string serialiseState();							// State -> JSON (to be send to PLC)
	void deserialiseState();								// JSON (from PLC) -> State 
	ExperimentParameters getExpParams();					// Retrieve experiment parameters from the State
	void setExpParams(ExperimentParameters newExpParams);	// Save experiment parameters to the State
protected:
	ExperimentParameters experimentParameters;
	int experimentProgressIndex = -1;
};


