	#pragma once
#include "DefaultExperimentSettings.h"
#include <vector>

struct ExperimentSettings {
	std::vector<double> Ts;
	int StartWL = 340;
	int DGrangeNo = 3;
	int averagingCount = 1;
	bool useBackgroundCorrection = false;
};
