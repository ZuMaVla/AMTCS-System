#pragma once
#include <vector>
#include <string>
#include "afxcmn.h"

const std::string default_SampleCode = "NT####";
const std::vector<int> extreme_Ts = {
	0, 450
};
const std::vector<int> extreme_Slits = {
	0, 2000
};
const std::vector<int> extreme_DGRangeNo = {
	1, 5
};
const std::vector<int> extreme_NA = {
	1, 16
};
const std::vector<int> extreme_StartWL = {
	200, 600
};
const std::vector<int> extreme_AT = {
	50, 5000
};

const int default_newT = 300;
const int default_DG = 2;			// 600 grooves/inch by default (Grating 3)
const int default_StartWL = 340;
const int default_DGRangeNo = 3;
const int default_NA = 4;
const int default_MaxAT = 250;
const std::vector<std::string> default_Ts = {
	"10", "20", "30", "40", "50", "60", "80", "100", "120", "150", "180", "220", "260", "300"
};
const bool default_isCRRemoval = TRUE;
const int default_Slits = 0;
