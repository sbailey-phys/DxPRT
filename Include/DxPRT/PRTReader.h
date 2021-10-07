/*
*
* Class to read in PRT files.
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

#pragma once

#include <Windows.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace DxPRT_Utility {

	/*
	* this struct used to store the indexed data, where each vertex constains
	* an index that can be used to read the spherical harmonic coefficients from
	* a buffer
	*/
	struct NumberedVertex 
	{
		float vertex[3];
		UINT32 index;
	};

	class PRTReader
	{
	public:

		//constructor that automatically call the load function
		PRTReader(const std::string &fileName, const bool &isEM = false);
		
		
		// default constructor
		PRTReader();


		/*
		* Load: reads the .prt file and stores the data in the relevant vectors
		* 
		* _IN_ fileName: path to the .prt file
		* _IN_ isEM: should be set to true if the file stores information about 
		*            an environment map
		*/
		bool Load(const std::string &fileName, const bool &isEM = false);

		// returns a pointer to the vertex data
		float* GetVertices();

		// returns a pointer to the index data
		UINT32* GetIndices();

		// returns a pointer to the coefficient data
		float* GetCoefficients();

		
		// returns a pointer to the index data (see the NumberedVertex struct)
		NumberedVertex* GetNumberedVertices();

		
		// returns the size of the vertex data (i.e. 3 * num vertices)
		size_t GetSizeVertices() const;

		
		// returns the size of the index data (i.e. 3 * num faces)
		size_t GetSizeIndices() const;


		// returns the size of the coefficient data
		size_t GetSizeCoefficients() const;


		// returns the maximum value of l for the spherical harmonics
		size_t GetMaxL() const;

		
		// returns the size of the indexde vertex data (i.e. 4 * num vertices)
		size_t GetSizeNumberedVertices() const;

		
		// get the total number of coefficients for each vertex
		int GetNCoefficients() const;

	private:

		/*
		* ProcessLine: processes a single line of the .prt file and passes it to the relevant
		* funtion depending on the specifier. Returns false if the read fails.
		*
		* _IN_ specifier: the character(s) specifing what data is contained in the line
		* _IN_ line: the data contained within the line
		*/
		bool ProcessLine(const std::string& specifier,
			const std::vector<std::string>& line);


		/*
		* ProcessLine: processes a single line of the .obj file and passes it to the relevant
		* funtion depending on the specifier. Designed for files containing an 
		* environment map. Returns false if the read fails.
		*
		* _IN_ specifier: the character(s) specifing what data is contained in the line
		* _IN_ line: the data contained within the line
		*/
		bool ProcessLineEM(const std::string& specifier,
			const std::vector<std::string>& line);


		/*
		* SetVertex: processes a line with the 'v' specifier. Returns false if failed
		*
		* _IN_ line: data in the line
		*/
		bool SetVertex(const std::vector<std::string>& line);
		
		/*
		* SetIndex: processes a line with the 'f' specifier. Returns false if failed
		*
		* _IN_ line: data in the line
		*/
		bool SetIndex(const std::vector<std::string>& line);
		
		// calculates the indexed vertex vector
		void CalcNumberedVertices();

		// throws an exception if the data is accessed before loading a file
		void NotLoadedMessage() const;

		bool numberedCalculated_ = false;
		std::vector<float> vertices_;
		std::vector<float> coefficients_;
		std::vector<UINT32> indices_;
		std::vector<NumberedVertex> numberedVertices_;
		size_t nCoefficients_ = 0, maxL_ = 0;

		bool isLoaded_ = false, maxLFound_ = false;
	};

}