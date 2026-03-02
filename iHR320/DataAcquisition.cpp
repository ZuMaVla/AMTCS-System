#include "stdafx.h"
#include "DataAcquisition.h"
#include "iHR320Dlg.h"
#include "ExperimentState.h"
#include "JYDeviceSink.h"
#include <fstream>
#include <string>

std::vector<long> CollectData(CiHR320Dlg* pUI) {
	std::vector<long> l_pixelData;
	if (!pUI->currentData.intensities.empty()) {
		l_pixelData = pUI->currentData.intensities;
	}
	return l_pixelData;
}

std::vector<double> CalculateData(const std::vector<std::vector<long>> &data, const std::vector<long> &bckg) {
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

std::vector<std::vector<long>> RepareData(std::vector<std::vector<long>> data) {
	return data;
}

std::vector<long> TakeBackground(CiHR320Dlg* pUI) {
	std::vector<long> l_backgroundData;
	if (!pUI->currentData.intensities.empty()) {
		l_backgroundData = pUI->currentData.intensities;
	}
	return l_backgroundData;
}

bool SaveData(const std::vector<double> &fullData, CString path, CString sampleCode, CString T) {
	CString filename = path + L"\\" + sampleCode + L"_" + T + L".xy";

	CFile outputFile;
	if (outputFile.Open(filename, CFile::modeCreate | CFile::modeWrite))
	{
		CString line;

		// Header
		line = L"Pixel\tCounts\n";
		outputFile.Write(CT2A(line), line.GetLength());

		// Data rows
		for (size_t i = 0; i < fullData.size(); i++) {
			line.Format(L"%zu\t%.4f\n", i, fullData[i]);
			outputFile.Write(CT2A(line), line.GetLength());
		}

		outputFile.Close();
		return true;
	}

	return false;
}

bool TakeSpectrum(CiHR320Dlg* pUI) {
	ExperimentParameters params = pUI->GetExperimentParameters();
	int NA = params.NA;
	int DGRangeNo = params.DGRangeNo;
	std::array<double, 5> centresWL = pUI->GetCentresWL(params.StartWL, params.DGRangeNo);
	std::vector<double> finalData, fullData;
	std::vector<long> bckg;
	std::vector<std::vector<long>> totalData;

	for (int i = 0; i < params.DGRangeNo; i++) {
		totalData.clear();
		pUI->MonoMoveTo(centresWL[i]);

		pUI->DoAcquisition(false);
		while (!pUI->m_isCCDDataReady) {
			Sleep(100);
		}
		bckg = TakeBackground(pUI);

		for (int j = 0; j < params.NA; j++) {
			pUI->DoAcquisition();
			while (!pUI->m_isCCDDataReady) {
				Sleep(100);
			}
			totalData.push_back(CollectData(pUI));
		}

		if (params.isCRRemoval) totalData = RepareData(totalData);			// Cosmic ray removal - TODO
		finalData = CalculateData(totalData, bckg);
		fullData.insert(fullData.end(), finalData.begin(), finalData.end());
	}

	CString path = pUI->GetCurrentDir();

	return SaveData(fullData, path, L"NT3809", L"200_K");
}