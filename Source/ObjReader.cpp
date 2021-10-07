/*
*
* Implimentation of the ObjReader Class (see ObjReader.h)
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

#include "DxPRT/ObjReader.h"


namespace DxPRT_Utility {

	ObjReader::ObjReader(const std::string &fileName, const bool &calculateNormals) {
		this->Load(fileName, calculateNormals);
	}

	ObjReader::ObjReader() : calcNormals_(false) {}


	bool ObjReader::Load(const std::string &fileName, const bool &calculateNormals)
	{
		std::ifstream infile(fileName);

		calcNormals_ = calculateNormals;

		while (infile)
		{
			std::string s;
			if (!getline(infile, s)) break;

			if (s[0] != '#' && s != "") // ignore comments and empty lines
			{
				std::istringstream ss(s);
				std::vector<std::string> stringVec;

				getline(ss, s, ' '); // get line specifier
				std::string specifier = s;

				while (ss)
				{
					std::string s;
					if (!getline(ss, s, ' ')) break;
					stringVec.push_back(s);
				}
				if (!this->ProcessLine(specifier, stringVec)) return false;

			}

		}

		// final checks
		if (vertices_.size() == 0 || vertices_.size() % 3 != 0) return false;
		if (indices_.size() == 0) return false;
		if (calcNormals_ && normals_.size() != vertices_.size()) return false;

		isLoaded_ = true;
		return true;
	}


	bool ObjReader::ProcessLine(const std::string& specifier,
		const std::vector<std::string>& line) 
	{

		if (specifier == "v") 
		{
			if (!this->SetVertex(line)) return false;
		}
		else if (specifier == "f") {
			if (!this->SetIndex(line)) return false;
			if (calcNormals_) 
			{
				if (!this->CalcNormal(line)) return false;
			}
		}
		return true;
	}

	bool ObjReader::SetVertex(const std::vector<std::string>& line)
	{
		if (line.size() != 3) return false;
		for (int i = 0; i < 3; ++i)
		{
			vertices_.push_back(std::stof(line[i]));
			if (calcNormals_) normals_.push_back(0.0f);
		}
		return true;
	}


	bool ObjReader::SetIndex(const std::vector<std::string>& line)
	{
		if (line.size() < 3) return false;
		for (int i = 0; i < 3; ++i) 
		{
			std::istringstream ss(line[i]);
			std::string vertexNum;
			getline(ss, vertexNum, '\\');
			indices_.push_back(std::stoi(vertexNum) - 1);
		}
		for (int i = 3; i < line.size(); ++i) // more that 3 vertices in face
		{
			std::istringstream ss(line[i]);
			std::string vertexNum;
			getline(ss, vertexNum, '\\');
			indices_.push_back(indices_[indices_.size() - 2]);
			indices_.push_back(indices_[indices_.size() - 2]);
			indices_.push_back(std::stoi(vertexNum) - 1);
		}
		return true;
	}

	bool ObjReader::CalcNormal(const std::vector<std::string>& line) {

		std::vector<int> indices;
		for (size_t i = 0; i < line.size(); ++i)
		{
			indices.push_back(stoi(line[i]) - 1);
			if ((size_t) indices[i] * 3 + 2 > vertices_.size())
			{
				return false; // index outside of vertex range
			}
		}

		for (size_t iFace = 0; iFace < line.size() - 2; ++iFace)
		{
			float vector1[3], vector2[3], normal[3];

			for (int i = 0; i < 3; ++i) {
				vector1[i] = vertices_[(size_t) indices[iFace + 1] * 3 + i] -
					vertices_[(size_t)indices[iFace] * 3 + i];
				vector2[i] = vertices_[(size_t)indices[iFace + 2] * 3 + i] -
					vertices_[(size_t)indices[iFace] * 3 + i];
			}

			normal[0] = vector1[1] * vector2[2] -
				vector1[2] * vector2[1];
			normal[1] = vector1[2] * vector2[0] -
				vector1[0] * vector2[2];
			normal[2] = vector1[0] * vector2[1] -
				vector1[1] * vector2[0];

			for (int iVertex = 0; iVertex < 3; ++iVertex) 
			{
				for (int iIndex = 0; iIndex < 3; ++iIndex) 
				{
					normals_[(size_t)indices[iFace + iIndex] * 3 + iVertex]
						+= normal[iVertex];
				}
			}
		}
		return true;
	}


	float* ObjReader::GetVertices() 
	{
		if (isLoaded_) 
		{
			return &vertices_[0];
		}
		else 
		{
			this->NotLoadedMessage();
			return nullptr;
		}
	}

	UINT32* ObjReader::GetIndices() 
	{
		if (isLoaded_)
		{
			return &indices_[0];
		}
		else 
		{
			this->NotLoadedMessage();
			return nullptr;
		}
	}

	float* ObjReader::GetNormals() 
	{
		if (isLoaded_ && calcNormals_)
		{
			return &normals_[0];
		}
		else
		{
			this->NotLoadedMessage();
			return nullptr;
		}

	}

	float* ObjReader::GetInterleaved()
	{
		if (isLoaded_ && calcNormals_)
		{
			if (!interleavedCalculated_) 
			{
				this->CalcInterleaved();
				interleavedCalculated_ = true;
			}
			return &interleaved_[0];
		}
		else 
		{
			this->NotLoadedMessage();
			return 0;
		}
	}

	size_t ObjReader::GetSizeVertices() const
	{
		if (isLoaded_) 
		{
			return vertices_.size();
		}
		else
		{
			this->NotLoadedMessage();
			return 0;
		}
	}
	size_t ObjReader::GetSizeIndices() const
	{
		if (isLoaded_)
		{
			return indices_.size();
		}
		else
		{
			this->NotLoadedMessage();
			return 0;
		}
	}

	size_t ObjReader::GetSizeNormals() const
	{
		if (isLoaded_ && calcNormals_)
		{
			return normals_.size();
		}
		else
		{
			this->NotLoadedMessage();
			return 0;
		}
	}

	size_t ObjReader::GetSizeInterleaved() const
	{
		if (isLoaded_)
		{
			if (interleavedCalculated_)
			{
				return interleaved_.size();
			}
			else
			{
				return vertices_.size() + normals_.size();
			}
		}
		else
		{
			this->NotLoadedMessage();
			return 0;
		}
	}

	void ObjReader::CalcInterleaved() 
	{
		auto iterV = vertices_.cbegin();
		auto iterN = normals_.cbegin();

		while (iterN != normals_.cend())
		{
			for (int i = 0; i < 3; ++i)
			{
				interleaved_.push_back(*iterV); // vertex
				++iterV;
			}
			for (int i = 0; i < 3; ++i)
			{
				interleaved_.push_back(*iterN); //normal
				++iterN;
			}
		}

	}


	void ObjReader::NotLoadedMessage() const 
	{
		OutputDebugStringA("DxPRT: Obj file is not loaded, cannot access data!\n");
		throw std::exception();
	}

}