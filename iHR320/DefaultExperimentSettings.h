#pragma once
#include <vector>
#include <string>
#include "afxcmn.h"

const std::string default_SampleCode = "NT####";
const std::vector<int> extreme_Ts = {			// temperatures
	0, 450
};
const std::vector<int> extreme_Slits = {		// slits width
	0, 2000
};
const std::vector<int> extreme_DGRangeNo = {	// diffraction grating
	1, 5
};
const std::vector<int> extreme_NA = {			// number of acquisitions
	1, 16
};
const std::vector<int> extreme_StartWL = {		// start wavelength
	200, 600
};
const std::vector<int> extreme_AT = {			// acquisition time
	50, 5000
};

const int HT = 320;							// High temperature threshold
const int default_newT = 300;
const int default_DG = 2;					// 600 grooves/inch by default (Grating 3)
const int default_StartWL = 340;
const int default_DGRangeNo = 3;
const int default_NA = 4;
const int default_MaxAT = 250;


const std::vector<std::string> default_Ts = {
	"11", "20", "30", "40", "50", "60", "80", "100", "120", "140", "160", "180", "200", "220", "240", "260", "280", "300"
};
const bool default_isCRRemoval = TRUE;
const int default_Slits = 0;

// General configurations:
const int minPhSlt = 10;					// Physically smallest slits
const int maxCCDI = int(std::pow(2, 14));	// Maximal allowed count from CCD
const int minCCDI = int(std::pow(2, 11));	// Minimal allowed count from CCD
const int HBRate = 60;						// Heartbeat rate in seconds