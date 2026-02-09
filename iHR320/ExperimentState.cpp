#include "stdafx.h"
#include "ExperimentState.h"


CExperimentState::CExperimentState()
{
}


CExperimentState::~CExperimentState()
{
}

void CExperimentState::importJSONString(const std::string& jsonString)
{
	nlohmann::json j;
	try
	{
		auto j = nlohmann::json::parse(jsonString);
	}
	catch (nlohmann::json::exception& e)
	{
		OutputDebugStringA(e.what());
	}
	jsonState = j;
}

std::string CExperimentState::serialiseState()
{
	jsonState["experimentParameters"] = experimentParameters;
	jsonState["experimentProgressIndex"] = experimentProgressIndex;
	return jsonState.dump();
}

void CExperimentState::deserialiseState()
{
	experimentParameters = jsonState.at("experimentParameters").get<ExperimentParameters>();
	experimentProgressIndex = jsonState.at("experimentProgressIndex").get<int>();
}

ExperimentParameters CExperimentState::getExpParams()
{
	return experimentParameters;
}

void CExperimentState::setExpParams(ExperimentParameters newExpParams)
{
	experimentParameters = newExpParams;
}
