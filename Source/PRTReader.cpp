/*
*
* Implimentation of the PRTReader Class (see PRTReader.h)
*
* Forms part of the DxPRT project
*
*
*
* Author: Shaun Bailey
*
* Date created: 04/10/2021
*
* Last Updated: 06/10/2021
*
*
*
* MIT License
*
* Copyright (c) 2021 Shaun Bailey
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
*/

#include "DxPRT/PRTReader.h"


namespace DxPRT_Utility {

	PRTReader::PRTReader(const std::string &fileName, const bool &isEM) {
		if (!this->Load(fileName, isEM))
		{
			OutputDebugStringA("DxPRT: Failed to load PRT file!\n file = ");
			OutputDebugStringA(fileName.c_str());
			OutputDebugStringA("\n");
		}
	}

	PRTReader::PRTReader() {}


	bool PRTReader::Load(const std::string &fileName, const bool &isEM) {

		std::ifstream infile(fileName);
		if (infile.fail()) return false;

		while (infile) {
			std::string s;
			if (!getline(infile, s)) break;

			// ignore comments and empty lines
			if (s[0] != '#' && s != "") {

				std::istringstream ss(s);
				std::vector<std::string> stringVec;

				// get line specifier
				getline(ss, s, ' ');
				std::string specifier = s;

				while (ss)
				{
					std::string s;
					if (!getline(ss, s, ' ')) break;
					stringVec.push_back(s);
				}

				if (isEM) {
					if (!this->ProcessLineEM(specifier, stringVec)) return false;
				}
				else {
					if (!this->ProcessLine(specifier, stringVec)) return false;
				}
			}

		}

		// final checks
		if (isEM) {
			if (coefficients_.size() != nCoefficients_ * 3) return false;
		}
		else {
			if (vertices_.size() == 0 || vertices_.size() % 3 != 0) return false;
			if (indices_.size() == 0) return false;
			if (coefficients_.size() != nCoefficients_ * vertices_.size() / 3) return false;
		}

		isLoaded_ = true;
		return true;

	}


	bool PRTReader::ProcessLine(const std::string& specifier,
		const std::vector<std::string>& line) {


		if (specifier == "v") {
			if (!maxLFound_) return false;
			if (!this->SetVertex(line)) return false;
		}
		else if (specifier == "f") {
			if (!this->SetIndex(line)) return false;
		}
		else if (specifier == "L") {
			if (maxLFound_ || line.size() != 1) return false;
			maxL_ = std::stoi(line[0]);
			nCoefficients_ = (maxL_ + 1) * (maxL_ + 1);
			maxLFound_ = true;
		}
		return true;
	}

	bool PRTReader::ProcessLineEM(const std::string& specifier,
		const std::vector<std::string>& line) {

		if (specifier == "L") {
			if (maxLFound_ || line.size() != 1) return false;
			maxL_ = std::stoi(line[0]);
			nCoefficients_ = (maxL_ + 1) * (maxL_ + 1);
			maxLFound_ = true;
		}
		else if (specifier == "c") {
			if (line.size() != nCoefficients_ * 3 || !maxLFound_) return false;
			for (auto iter = line.cbegin(); iter != line.cend(); ++iter) {
				coefficients_.push_back(std::stof(*iter));
			}
		}

		return true;
	}

	bool PRTReader::SetVertex(const std::vector<std::string>& line) {

		if (line.size() != 3 + nCoefficients_ || !maxLFound_) return false;

		for (int i = 0; i < 3; ++i) {
			vertices_.push_back(std::stof(line[i]));
		}
		for (int i = 0; i < nCoefficients_; ++i) {
			coefficients_.push_back(std::stof(line[3 + i]));
		}

		return true;

	}

	bool PRTReader::SetIndex(const std::vector<std::string>& line) {
		if (line.size() != 3 || !maxLFound_) return false;

		for (int i = 0; i < 3; ++i) {
			indices_.push_back(std::stoul(line[i]));
		}
		return true;
	}

	float* PRTReader::GetVertices() {
		if (isLoaded_) return &vertices_[0];
		else {
			this->NotLoadedMessage();
			return nullptr;
		}
	}


	float* PRTReader::GetCoefficients() {
		if (isLoaded_) return &coefficients_[0];
		else {
			this->NotLoadedMessage();
			return nullptr;
		}
	}

	UINT32* PRTReader::GetIndices() {
		if (isLoaded_) return &indices_[0];
		else {
			this->NotLoadedMessage();
			return nullptr;
		}
	}


	NumberedVertex* PRTReader::GetNumberedVertices() {
		if (isLoaded_) {
			if (!numberedCalculated_) {
				this->CalcNumberedVertices();
				numberedCalculated_ = true;
			}
			return &numberedVertices_[0];
		}
		else {
			this->NotLoadedMessage();
			return nullptr;
		}
	}

	size_t PRTReader::GetSizeVertices() const{
		if (isLoaded_) return vertices_.size();
		else {
			this->NotLoadedMessage();
			return 0;
		}
	}
	size_t PRTReader::GetSizeIndices() const{
		if (isLoaded_) return indices_.size();
		else {
			this->NotLoadedMessage();
			return 0;
		}
	}

	size_t PRTReader::GetSizeCoefficients() const{
		if (isLoaded_) return coefficients_.size();
		else {
			this->NotLoadedMessage();
			return 0;
		}
	}

	size_t PRTReader::GetSizeNumberedVertices() const{
		if (isLoaded_) {
			if (numberedCalculated_) {
				return numberedVertices_.size();
			}
			else {
				return (vertices_.size() / 3);
			}
		}
		else {
			this->NotLoadedMessage();
			return 0;
		}
	}

	size_t PRTReader::GetMaxL() const{
		if (isLoaded_) return maxL_;
		else {
			this->NotLoadedMessage();
			return 0;
		}
	}


	int PRTReader::GetNCoefficients() const{
		if (isLoaded_) return nCoefficients_;
		else {
			this->NotLoadedMessage();
			return 0;
		}
	}


	void PRTReader::CalcNumberedVertices() {
		auto iterV = vertices_.cbegin();
		UINT32 iData = 0;
		// add vertices
		while (iterV != vertices_.cend()) {
			NumberedVertex numberedVertex;
			for (int i = 0; i < 3; ++i) {
				numberedVertex.vertex[i] = (*iterV);
				++iterV;
			}
			numberedVertex.index = iData;
			numberedVertices_.push_back(numberedVertex);
			++iData;
		}

	}



	void PRTReader::NotLoadedMessage() const{
		OutputDebugStringA("DxPRT: PRT file is not loaded, cannot access data!\n");
		throw std::exception();
	}

}