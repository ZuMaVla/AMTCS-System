#include "stdafx.h"
#include "DataAcquisition.h"
#include "iHR320Dlg.h"
#include "ExperimentState.h"
#include "JYDeviceSink.h"
#include <fstream>
#include <string>
#include <iostream>

std::vector<long> CollectData(CiHR320Dlg* pUI) {
	std::vector<long> l_pixelData;
	if (!pUI->currentData.intensities.empty()) {
		l_pixelData = pUI->currentData.intensities;
	}
	return l_pixelData;
}

std::vector<double> CalculateData(const std::vector<std::vector<long>> &data, const std::vector<double> &bckg) {
	if (data.empty()) return{};
	

	
	std::vector<double> l_averagedData(data[0].begin(), data[0].end());	// Start with a copy of the first accumulation
	size_t numPixels = l_averagedData.size();
	size_t numAccums = data.size();
	if (numAccums == 1 && !bckg.empty()) {
		for (size_t p = 0; p < numPixels; p++) {
			if (bckg.size() == numPixels) {
				l_averagedData[p] -= bckg[p];		// Subtract background signal pixel by pixel
			}
		}
		return l_averagedData;
	}

	// 1. Sum all remaining accumulations starting from index 1 as 0th is already there
	for (size_t i = 1; i < numAccums; i++) {
		for (size_t p = 0; p < numPixels; p++) {
			l_averagedData[p] += data[i][p];
		}
	}

	// 2. Average and Subtract Background
	for (size_t p = 0; p < numPixels; p++) {
		l_averagedData[p] /= numAccums; // Taking average

		if (!bckg.empty() && bckg.size() == numPixels) {
			l_averagedData[p] -= bckg[p]; // Subtract dark frame
		}
	}

	return l_averagedData;
}

double SkippedAvg(const std::vector<long> &data1D, int skippedIndex = -1) {
	size_t numAccums = data1D.size();
	if (numAccums < 2 && skippedIndex != -1) {
		return SkippedAvg(data1D);						// Downgrade to standard average
	}

	double avg = 0.0;
	for (size_t r = 0; r < numAccums; r++) {
		if (r != skippedIndex) avg += data1D[r];
	}
	if (skippedIndex == -1 || skippedIndex >= numAccums) avg /= numAccums;
	else avg = avg/(numAccums - 1);

	return avg;
}

std::vector<long> Repair1D(std::vector<long> data1D) {
	size_t numAccums = data1D.size();
	double avg = SkippedAvg(data1D);
	int CRIndex = -1;
	for (size_t r = 0; r < numAccums; r++) {
		if (abs(data1D[r] - avg) / max(avg, 320) > 0.2) {
			CRIndex = r;
		}
	}
	if (CRIndex != -1) {
		std::cout << "Cosmic ray pixel detected.\n";
		avg = SkippedAvg(data1D, CRIndex);
		data1D[CRIndex] = static_cast<long>(round(avg));
		return Repair1D(data1D);
	}
	else { return data1D; }
}

std::vector<std::vector<long>> RepareData(std::vector<std::vector<long>> data) {
	size_t numPixels = data[0].size();
	size_t numAccums = data.size();
	if (numAccums < 3) { return data; }
	std::vector<long> column(numAccums), repairedColumn(numAccums);

	for (size_t c = 0; c < numPixels; c++)
	{
		for (size_t r = 0; r < numAccums; r++) {
			column[r] = data[r][c];
		}
		repairedColumn = Repair1D(column);
		for (size_t r = 0; r < numAccums; r++) {
			data[r][c] = repairedColumn[r];
		}
	}
	std::cout << "Repaired for cosmic rays. \n";
	return data;
}

std::vector<long> TakeBackground(CiHR320Dlg* pUI) {
	std::vector<long> l_backgroundData;
	if (!pUI->currentData.intensities.empty()) {
		l_backgroundData = pUI->currentData.intensities;
	}
	return l_backgroundData;
}

std::vector<double> CollectX(CiHR320Dlg* pUI) {
	std::vector<double> l_xData;
	if (!pUI->currentData.wavelengths.empty()) {
		l_xData = pUI->currentData.wavelengths;
	}
	return l_xData;
}

bool SaveData(
	const std::vector<double> &fullX, 
	const std::vector<double> &fullData, 
	CString path,
	CString sampleCode,
	CiHR320Dlg* pUI,
	CString T) {
	CString apcStr;
	apcStr.Format(_T("%02d"), pUI->acqParams.paramCount);
	CString filename = path + L"\\" + sampleCode + L"_" + T + L"_" + apcStr + L".xy";

	CFile outputFile;
	if (outputFile.Open(filename, CFile::modeCreate | CFile::modeWrite))
	{
		CString line;

		// Header
		line = _T("Wavelength\t\\i(I)\\-(PL)\n");
		outputFile.Write(CT2A(line), line.GetLength());
		line = _T("nm\tCPS\n");
		outputFile.Write(CT2A(line), line.GetLength());
		line = _T("\t") + sampleCode + _T("\n");
		outputFile.Write(CT2A(line), line.GetLength());
		line.Format(_T("AT: %04d\tslits: %04d\n"), pUI->acqParams.AT, pUI->acqParams.slits);
		outputFile.Write(CT2A(line), line.GetLength());


		// Data rows
		for (size_t i = 0; i < fullData.size(); i++) {
			line.Format(L"%.4f\t%.4f\n", fullX[i], fullData[i]);
			outputFile.Write(CT2A(line), line.GetLength());
		}
		outputFile.Close();
		return true;
	}

	return false;
}

bool TakeSpectrum(CiHR320Dlg* pUI, CString T) {
	ExperimentParameters params = pUI->GetExperimentParameters();
	bool success = false;
	int NA = params.NA;
	int DGRangeNo = params.DGRangeNo;
	std::array<double, 5> centresWL = pUI->GetCentresWL(params.StartWL, params.DGRangeNo);
	std::vector<double> zero, bckg, finalData, fullData, finalX, fullX;
	std::vector<std::vector<long>> bckgData, totalData;

	if (pUI->acqParams.AT == -1 || pUI->acqParams.slits == -1) {
		pUI->acqParams.AT = params.maxAT;
		pUI->acqParams.slits = params.slits;
	}
	pUI->SetSlits(pUI->acqParams.slits / 1000.0);
	pUI->SetAT(pUI->acqParams.AT / 1000.0);


	for (int j = 0; j < 2*params.NA; j++) {
		pUI->DoAcquisition(false);
		while (!pUI->m_isCCDDataReady) {
			Sleep(100);
		}
		bckgData.push_back(CollectData(pUI));
	}
	if (params.isCRRemoval) bckgData = RepareData(bckgData);			// Cosmic ray removal - TODO
	bckg = CalculateData(bckgData, zero);


	for (int i = 0; i < params.DGRangeNo; i++) {
		totalData.clear();
		pUI->MonoMoveTo(centresWL[i]);

		for (int j = 0; j < params.NA; j++) {
			pUI->DoAcquisition();
			while (!pUI->m_isCCDDataReady) {
				Sleep(100);
			}
			totalData.push_back(CollectData(pUI));
		}

		if (params.isCRRemoval) totalData = RepareData(totalData);			// Cosmic ray removal - TODO
		finalData = CalculateData(totalData, bckg);
		finalX = CollectX(pUI);
		fullData.insert(fullData.end(), finalData.begin(), finalData.end());
		fullX.insert(fullX.end(), finalX.begin(), finalX.end());
	}

	long maxIntensity = *std::max_element(fullData.begin(), fullData.end());

	CString path = pUI->GetCurrentDir();
	CString sampleCode = CString(params.sampleCode.c_str());
	success = SaveData(fullX, fullData, path, sampleCode, pUI, T);
	std::cout << "Max intensity: " << maxIntensity << "\n";
	if (params.isCRRemoval && maxIntensity > maxCCDI) {
		if (pUI->acqParams.AdjustAcqParam(params.maxAT, params.slits, 1.5*minCCDI/maxIntensity)) success = TakeSpectrum(pUI, T);
	}
	else if (params.isCRRemoval && maxIntensity < minCCDI) {
		if (pUI->acqParams.AdjustAcqParam(params.maxAT, params.slits, 0.75*maxCCDI/maxIntensity)) success = TakeSpectrum(pUI, T);
	}
	return success;
}



AcquisitionParameters::AcquisitionParameters()
{
}


AcquisitionParameters::~AcquisitionParameters()
{
}

bool AcquisitionParameters::AdjustAcqParam(int maxAT, int maxSlits, double factor)
{
	std::cout << "Adjustment requested with the following factor: " << factor << "\n";

	if (factor <= 1 && AT == extreme_AT[0] && slits == extreme_Slits[0]) return false;		// Phisically possible minima 
	if (factor >= 1 && AT == maxAT && slits == maxSlits) return false;						// User defined maxima
	if (factor > 1) {
		if (maxAT / AT >= factor) {
			AT = max(50, int(AT*factor));						
		}
		else {
			factor = factor*maxAT/AT;						// Residual factor (decreased proportionally to AT change)
			AT = maxAT;										// Increase AT as much as possible
			if (maxSlits / max(slits, minPhS) >= factor) {
				slits = int(max(slits, minPhS)*factor);		// Increase slits propotional to the residual factor
			}
			else {
				slits = maxSlits;							// Increase slits as much as possible
			}
		}
	}
	else {
		if (minPhS / max(slits, minPhS) <= factor) {
			slits = int(max(slits, minPhS)*factor);			// Decrease slits propotional to factor
			if (slits < minPhS) slits = extreme_Slits[0];
		}
		else {
			factor = factor*max(slits, minPhS) / minPhS;	// Residual factor (increased proportionally to slits change)
			slits = extreme_Slits[0];						// Decrease slits as much as possible
			if (extreme_AT[0] / AT <= factor) {
				AT = max(50, int(AT*factor));				// Decrease AT propotional to factor
			}
			else {
				AT = extreme_AT[0];							// Decrease AT as much as possible
			}
		}

	}
	paramCount++;
	return true;
}
