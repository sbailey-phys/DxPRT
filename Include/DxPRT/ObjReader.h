/*
*
* Class to read in data from an obj file.
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
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>


namespace DxPRT_Utility {

	class ObjReader
	{
	public:

		// constructor that automatically calls the Load function
		ObjReader(const std::string &fileName, const bool &calculateNormals = true);
		
		// default constructor
		ObjReader();

		/*
		* Load: reads in the data from the the .obj file and stores the data in the relavent
		* vectors
		* 
		* _IN_ fileName: the path to the .obj file
		* _IN_ calculateNormals: if set to true, then the normal at each vertex will be calculated
		*/
		bool Load(const std::string &fileName, const bool &calculateNormals = true);

		
		//returns a pointer to the vertex data
		float* GetVertices();

		// returns a pointer to the index data
		UINT32* GetIndices();

		// returns a pointer to the normal data (calculate normals must have been set to true)
		float* GetNormals();

		// returns a pointer to a vector containing both the vertex and normal data interleaved
		// (calculate normals must have been set to true)
		float* GetInterleaved();

		// returns the size of the vertex data (i.e. 3 * num vertices)
		size_t GetSizeVertices() const;

		// returns the size of the index data (i.e. 3 * num triangles)
		size_t GetSizeIndices() const;

		// returns the size of the normal data (i.e. 3 * num vertices)
		size_t GetSizeNormals() const;

		// returns the size of the interleaved data (i.e. 6 * num vertices)
		size_t GetSizeInterleaved() const;


	private:

		/*
		* ProcessLine: processes a single line of the .obj file and passes it to the relevant
		* funtion depending on the specifier. Returns false if the read fails.
		* 
		* _IN_ specifier: the character(s) specifing what data is contained in the line
		* _IN_ line: the data contained within the line
		*/
		bool ProcessLine(const std::string& specifier,
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

		/*
		* CalcNormal: calculates the normal to a paritcular face and adds it to the normals
		* at each vertex such that after all faces are process, each vertex will have the
		* correct normal
		*
		* _IN_ line: data in the line
		*/
		bool CalcNormal(const std::vector<std::string>& line);

		//Calculates the interleaved vector
		void CalcInterleaved();

		// produces an error if data is accessed when the object is not loaded
		void NotLoadedMessage() const;

		bool interleavedCalculated_ = false;
		bool calcNormals_;
		std::vector<float> vertices_;
		std::vector<float> normals_;
		std::vector<float> interleaved_;
		std::vector<UINT32> indices_;

		bool isLoaded_ = false;
	};

}