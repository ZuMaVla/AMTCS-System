#pragma once
#include <vector>
#include "afxcmn.h"


const std::vector<int> ExtremeTs = {
	0, 450
};
const std::vector<int> ExtremeSlits = {
	0, 2000
};
const std::vector<int> ExtremeDGRangeNo = {
	1, 5
};
const std::vector<int> ExtremeNA = {
	1, 16
};
const std::vector<int> ExtremeStartWL = {
	200, 600
};

const int default_newT = 300;
const int default_DG = 1;
const int default_StartWL = 340;
const int default_DGRangeNo = 3;
const int default_NA = 4;
const std::vector<std::string> DefaultTs = {
	"10", "20", "30", "40", "50", "60", "80", "100", "120", "150", "180", "220", "260", "300"
};
const bool default_isCRRemoval = TRUE;
const int default_Slits = 0;
