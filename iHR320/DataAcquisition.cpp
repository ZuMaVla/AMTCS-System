#include "stdafx.h"
#include "DataAcquisition.h"
#include "iHR320Dlg.h"
#include "ExperimentState.h"

void CollectData(int measNo) {

}; 

void CalculateData() {

};

void RepareData() {

};

void TakeBackground() {

};

bool SaveData() {
	return true;
};


bool TakeSpectrum(CiHR320Dlg* pUI) {
	ExperimentParameters params = pUI->GetExperimentParameters();
	int NA = params.NA;
	int DGRangeNo = params.DGRangeNo;
	std::array<double, 5> centresWL = pUI->GetCentresWL(params.StartWL, params.DGRangeNo);
	for (int i = 0; i < params.DGRangeNo; i++) {
		pUI->MonoMoveTo(centresWL[i]);

		pUI->DoAcquisition(false);
		TakeBackground();
		for (int j = 0; j < params.NA; j++) {
			pUI->DoAcquisition();
			CollectData(j);
		}
		if (params.isCRRemoval) RepareData();
		CalculateData();

	}
	
	return SaveData();
};

