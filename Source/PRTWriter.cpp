/*
*
* Implimentation of the PRTWritter Class (see PRTWriter.h)
*
* Forms part of the DxPRT project
*
*
*
* Author: Shaun Bailey
*
* Date created: 04/10/2021
*
* Last Updated: 04/10/2021
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

#include "DxPRT/PRTWriter.h"


namespace DxPRT_Utility {

	PRTWriter::PRTWriter() :
		pVertex_(nullptr), pIndex_(nullptr), pCoefficient_(nullptr),
		vertexSize_(0), indexSize_(0), coefficientSize_(0),
		nCoefficients_(0), maxL_(0) {}

	bool PRTWriter::Write(const std::string &filename, const bool &isEM) {

		if (isEM && !addedCoefficients_) return false;
		if (!isEM && (!addedCoefficients_ || !addedIndices_ ||
			!addedVertices_)) return false;

		std::ofstream outFile;

		outFile.open(filename);
		if (outFile.fail()) return false;


		outFile << "L " << maxL_ << "\n";
		if (isEM) {
			this->writeCoefficients(outFile);
		}
		else {
			this->writeVertices(outFile);
			this->writeIndices(outFile);
		}
		outFile.close();

		return true;
	}




	void PRTWriter::AddVertices(float* pVertex, const size_t &size) {
		pVertex_ = pVertex;
		vertexSize_ = size;
		addedVertices_ = true;
	}

	void PRTWriter::AddCoefficients(const int& maxL, float* pCoefficient, const size_t &size) {
		maxL_ = maxL;
		nCoefficients_ = (maxL + 1) * (maxL + 1);
		pCoefficient_ = pCoefficient;
		coefficientSize_ = size;
		addedCoefficients_ = true;
	}

	void PRTWriter::AddIndices(UINT32* pIndex, const size_t &size) {
		pIndex_ = pIndex;
		indexSize_ = size;
		addedIndices_ = true;
	}

	void PRTWriter::writeVertices(std::ofstream& file) const {
		for (int i = 0; i < vertexSize_ / 3; ++i) {
			file << "v";
			for (int j = 0; j < 3; ++j) {
				file << " " << *(pVertex_ + 3 * i + j);
			}
			for (int j = 0; j < nCoefficients_; ++j) {
				file << " " << *(pCoefficient_ + i * nCoefficients_ + j);
			}
			file << "\n";
		}
	}

	void PRTWriter::writeIndices(std::ofstream& file) const {
		for (int i = 0; i < indexSize_ / 3; ++i) {
			file << "f";
			for (int j = 0; j < 3; ++j) {
				file << " " << *(pIndex_ + 3 * i + j);
			}
			file << "\n";
		}
	}


	void PRTWriter::writeCoefficients(std::ofstream& file) const {
		file << "c ";
		for (int j = 0; j < nCoefficients_ * 3; ++j) {
			file << *(pCoefficient_ + j);
			if (j != nCoefficients_ * 3 - 1) file << " ";
		}
	}

}