/*
*
* Implimentation of the HDRReader Class (see HDRReader.h)
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

#include "DxPRT/HDRReader.h"


namespace DxPRT_Utility {

	HDRReader::HDRReader() {}

	HDRReader::HDRReader(const std::string &fileString) 
	{
		this->Load(fileString);
	}

	bool HDRReader::Load(const std::string &fileString) 
	{

		std::ifstream infile(fileString, std::ifstream::binary);
		if (infile.fail()) return false;

		if (!this->ProcessHeader(infile)) return false;

		std::vector<float> decodedData;
		if (!this->DecodeByteCode(infile, decodedData)) return false;

		this->GenerateData(decodedData);

		infile.close();

		isLoaded_ = true;

		return true;
	}


	float* HDRReader::GetData() 
	{
		if (isLoaded_) return &data_[0];
		else
		{
			this->NotLoadedMessage();
			return nullptr;
		}
	}


	size_t HDRReader::GetNPixels() const 
	{
		if (isLoaded_) return data_.size() / 3;
		else
		{
			this->NotLoadedMessage();
			return 0;
		}
	}


	size_t HDRReader::GetNPixelsX() const 
	{
		if (isLoaded_) return width_;
		else
		{
			this->NotLoadedMessage();
			return 0;
		}
	}
	size_t HDRReader::GetNPixelsY() const 
	{
		if (isLoaded_) return height_;
		else
		{
			this->NotLoadedMessage();
			return 0;
		}
	}



	bool HDRReader::ProcessHeader(std::ifstream& infile)
	{

		bool inHeader = true;
		std::string s;

		getline(infile, s);

		if (s.substr(0, 2) != "#?") return false; // indicates that this is a radiance file

		// find end of main header, all settings are ignored
		while (inHeader) 
		{
			getline(infile, s);
			if (s == "") 
			{
				inHeader = false;
			}
		}

		// get width and height
		// current itteration only accepts -Y N +X M
		getline(infile, s);
		std::istringstream ss(s);

		getline(ss, s, ' ');
		if (s != "-Y") return false;
		getline(ss, s, ' ');
		height_ = std::stoi(s);

		getline(ss, s, ' ');
		if (s != "+X") return false;
		getline(ss, s, ' ');
		width_ = std::stoi(s);

		return true;
	}

	bool HDRReader::DecodeByteCode(std::ifstream& infile,
		std::vector<float>& decodedData) const
	{

		auto startPos = infile.tellg(); // get position in data
		infile.seekg(0, std::ios::end);
		auto endPos = infile.tellg();
		auto byteSize = endPos - startPos;
		infile.seekg(startPos);

		std::vector<unsigned char> byteData(byteSize, 0);

		infile.read((char*)&byteData[0], byteSize);
		auto iter = byteData.cbegin();
		auto iterEnd = byteData.cend();

		decodedData.resize(width_ * height_ * 4ull);
		int index = 0;

		// loop over data
		for (unsigned int iLine = 0; iLine < height_; ++iLine) 
		{
			iter += 4; // skip first 4 bytes
			unsigned int iData = 0;

			while (iData < width_ * 4) 
			{
				unsigned int counter = (unsigned int)(*iter);

				
				if (counter > 128) // run of the same number
				{
					counter -= 128;
					if (iterEnd - iter < 2) {
						return false; //bad scanline data
					}
					++iter;
					for (unsigned int i = 0; i < counter; ++i) {
						decodedData[index] = (float)(*iter);
						++index;
					}
				}
				else // run of different numbers
				{
					if (iterEnd - iter < counter + 1ull) {
						return false; //bad scanline data
					}
					for (unsigned int i = 0; i < counter; ++i) {
						decodedData[index] = (float)(*(++iter));
						++index;
					}
				}
				++iter;

				
				if (width_ * 4 - iData < counter) // ensure that there is enough bytes left for this line
				{
					return false; //bad scanline data
				}

				iData += counter;
			}
		}

		return true;
	}


	void HDRReader::GenerateData(const std::vector<float>& decodedData)
	{
		data_.resize(width_ * height_ * 3ull);
		for (size_t i = 0; i < height_; ++i) 
		{
			for (size_t j = 0; j < width_; ++j) 
			{
				float red = (float)(decodedData[width_ * 4ull * i + j]);
				float green = (float)(decodedData[width_ * (4ull * i + 1ull) + j]);
				float blue = (float)(decodedData[width_ * (4ull * i + 2ull) + j]);
				float exp = (float)(decodedData[width_ * (4ull * i + 3ull) + j]);


				data_[(i * width_ + j) * 3ull] = (red + 0.5f) * powf(2.0f, exp - 128.0f) / 256.0f;
				data_[(i * width_ + j) * 3ull + 1ull] = (green + 0.5f) * powf(2.0f, exp - 128.0f) / 256.0f;
				data_[(i * width_ + j) * 3ull + 2ull] = (blue + 0.5f) * powf(2.0f, exp - 128.0f) / 256.0f;

			}
		}
	}


	void HDRReader::NotLoadedMessage() const 
	{
		OutputDebugStringA("DxPRT: HDR file is not loaded, cannot access data!\n");
		throw std::exception();
	}

}