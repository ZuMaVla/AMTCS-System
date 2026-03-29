	#pragma once
#include "stdafx.h"
#include "DefaultExperimentSettings.h"
#include <vector>
#include <json.hpp>

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

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(
	ExperimentParameters,
	sampleCode,
	Ts, 
	StartWL,
	DG, 
	DGRangeNo,
	NA, 
	slits,
	maxAT,
	isCRRemoval
)

class CExperimentState
{
public:
	CExperimentState();
	~CExperimentState();

	void importJSONString(const std::string & jsonString);
	int experimentProgressIndex = -1;
	int experimentLength = experimentParameters.Ts.size();

	std::string serialiseState();							// State -> JSON (to be send to PLC)
	void deserialiseState();								// JSON (from PLC) -> State 
	ExperimentParameters getExpParams();					// Retrieve experiment parameters from the State
	void setExpParams(ExperimentParameters newExpParams);	// Save experiment parameters to the State
protected:
	ExperimentParameters experimentParameters;
	nlohmann::json jsonState;
	NLOHMANN_DEFINE_TYPE_INTRUSIVE(
		CExperimentState,
		experimentParameters,
		experimentProgressIndex,
		experimentLength
	)
};




