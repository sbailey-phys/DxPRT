/*
*
* Class used to read in .hdr files that are of the RGBE format and are run-length 
* encoded. In the current itteration, it is expected that the hdr file will have
* phi along the x-axis and theta along the y-axis.
*
*
* This class is part of the implimentation and is not intended for public
* use.
*
*
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

#pragma once

#include<string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <Windows.h>
#include <exception>


namespace DxPRT_Utility {

	class HDRReader
	{
	public:

		// default initializer
		HDRReader();

		// this initializer automatically calls the Load function
		HDRReader(const std::string &fileString);

		/*
		* Load: reads the hdr file and stores the relevant data. If the file cannot be read
		* then the function will return false.
		* 
		* _IN_ fileString: path to the hdr file
		*/
		bool Load(const std::string &fileString);

		/*
		* GetData: returns a pointer to the hdr data. Each pixel is contains 3 floats for
		* the RGB channels. The pixels are stored left-to-right then top-to-bottom
		*/
		float* GetData();
		
		// returns the width of the image
		size_t GetNPixelsX() const;

		// returns the height of the image
		size_t GetNPixelsY() const;

		// return the total number of pixels
		size_t GetNPixels() const;

	private:

		/*
		* ProcessHeader: reads the header of the hdr file. Returns false if it fails
		* 
		* _IN/OUT_ infile: the current file stream
		*/
		bool ProcessHeader(std::ifstream& infile);

		/*
		* DecodeByteCode: decodes the run-length encoded data into the full RGBE data
		* Returns false if it fails.
		* 
		* _IN/OUT_ infile: the current file stream
		* _OUT_ decodedData: vector containing the decoded data, stored as floats
		*/
		bool DecodeByteCode(std::ifstream& infile,
			std::vector<float>& decodedData) const;

		/*
		* GenerateData: transforms the data from the full RGBE format to the final hdr
		* RGB format
		* 
		* _IN_ decodedData: the full RGBE data
		*/
		void GenerateData(const std::vector<float>& decodedData);


		// returns an exception if the data is access before the file is loaded
		void NotLoadedMessage() const;

		std::vector<float> data_;
		size_t lineNumber_ = 0;
		size_t height_ = 0;
		size_t width_ = 0;

		bool isLoaded_ = false;

	};

}