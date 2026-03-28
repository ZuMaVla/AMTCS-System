#include "stdafx.h"
#include "ExperimentState.h"
#include <iostream>
#include <iomanip>   // for std::hex, std::dec
#include <cctype>    // for std::isprint


CExperimentState::CExperimentState()
{
}

CExperimentState::~CExperimentState()
{
}

void CExperimentState::importJSONString(const std::string& jsonString)
{
	// Raw bytes exactly as received (for debugging/logging)
	std::cout << "RAW JSON RECEIVED (size=" << jsonString.size() << "):\n"; 

	for (unsigned char c : jsonString)
	{
		if (std::isprint(c))
			std::cout << c;				// Print as is if printable
		else
			std::cout << "\\x" << std::hex << std::setw(2) << std::setfill('0') << (int)c << std::dec;
	}
	std::cout << "\n";

	try
	{
		jsonState = nlohmann::json::parse(jsonString);
		std::cout << "PARSED TYPE: " << jsonState.type_name() << "\n";
	}
	catch (const nlohmann::json::exception& e)
	{
		std::cout << "JSON PARSE ERROR: " << e.what() << "\n";
	}
}


std::string CExperimentState::serialiseState()
{
	jsonState["experimentParameters"] = experimentParameters;
	jsonState["experimentProgressIndex"] = experimentProgressIndex;
	jsonState["experimentLength"] = experimentLength;
	return jsonState.dump();
}

void CExperimentState::deserialiseState()
{
	experimentParameters = jsonState.at("experimentParameters").get<ExperimentParameters>();
	experimentProgressIndex = jsonState.at("experimentProgressIndex").get<int>();
	experimentLength = jsonState.at("experimentLength").get<int>();
}

ExperimentParameters CExperimentState::getExpParams()
{
	return experimentParameters;
}

void CExperimentState::setExpParams(ExperimentParameters newExpParams)
{
	experimentParameters = newExpParams;
}
