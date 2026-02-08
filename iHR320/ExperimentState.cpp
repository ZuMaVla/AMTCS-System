#include "stdafx.h"
#include "ExperimentState.h"


CExperimentState::CExperimentState()
{
}


CExperimentState::~CExperimentState()
{
}

std::string CExperimentState::serialiseState()
{
	return std::string();
}

void CExperimentState::deserialiseState()
{
}

ExperimentParameters CExperimentState::getExpParams()
{
	return experimentParameters;
}

void CExperimentState::setExpParams(ExperimentParameters newExpParams)
{
	experimentParameters = newExpParams;
}
