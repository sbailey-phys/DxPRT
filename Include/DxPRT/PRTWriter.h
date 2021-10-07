/*
*
* Class to write a prt file
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

#include <Windows.h>
#include <iostream>
#include <fstream>
#include <string>


namespace DxPRT_Utility {

	class PRTWriter
	{

	public:

		// default constructor
		PRTWriter();

		/*
		* writes out the data to a file. returns false if there is not enough
		* data or cannot access file
		* 
		* _IN_ fileName: path to the file
		* _IN_ isEM: if set to true, then the output file will contain an environment
		*            map
		*/
		bool Write(const std::string &filename, const bool &isEM = false);


		/*
		* AddVertices: sets the pointer to the vertex data
		* 
		* _IN_ pVertex: pointer to the vertex data
		* _IN_ size_t: number of elements in the vertex data (i.e. 3 * nuumber of vertices)
		*/
		void AddVertices(float* pVertex, const size_t &size);

		/*
		* AddCoefficients: sets the pointer to the coefficient data
		*
		* _IN_ maxL: the maximum value of l for the sphierical harmonic coefficients
		* _IN_ pVertex: pointer to the coefficient data
		* _IN_ size_t: number of elements in the coefficient data (i.e. (maxL+1)^2*number of vertices)
		*/
		void AddCoefficients(const int& maxL, float* pCoefficient, const size_t &size);


		/*
		* AddIndices: sets the pointer to the index data
		*
		* _IN_ pVertex: pointer to the index data
		* _IN_ size_t: number of elements in the index data (i.e. 3 * nuumber of indices)
		*/
		void AddIndices(UINT32* pIndex, const size_t &size);

	private:

		/*
		* writeVertices: writes a line containing information about a vertex (both position
		* and coefficients)
		* 
		* _IN/OUT_ file: the file being written
		*/
		void writeVertices(std::ofstream& file) const;


		/*
		* writeIndices: writes a line containing information about a face
		*
		* _IN/OUT_ file: the file being written
		*/
		void writeIndices(std::ofstream& file) const;

		/*
		* writeIndices: writes a line containing coefficients for an environment map
		*
		* _IN/OUT_ file: the file being written
		*/
		void writeCoefficients(std::ofstream& file) const;

		float* pVertex_, * pCoefficient_;
		UINT32* pIndex_;
		size_t vertexSize_, coefficientSize_, indexSize_;
		size_t nCoefficients_, maxL_;

		bool addedVertices_ = false, addedIndices_ = false,
			addedCoefficients_ = false;

	};

}